#include"../include/ChatGPTProvider.h"
#include"../include/until/mylog.h"
#include<jsoncpp/json/json.h>
#include<sstream>

namespace ai_chat_sdk
{
    bool ChatGPTProvider::initModel(const std::map<std::string , std::string>& modelConfig)
    {
            //配置apiKey
            auto apikey = modelConfig.find("api_Key");
            if(apikey == modelConfig.end())
            {
                ERR("api_Key is not found in modelConfig");
                return false;
            }
            _apiKey = apikey->second;
            
            //配置Base URL
            auto baseUrl = modelConfig.find("base_URL");
            if(baseUrl != modelConfig.end())
            {
                _endPoint = baseUrl->second;
            }
            _endPoint = "https://api.openai.com";

            _isAvailable = true;
            return true;
    }
    bool ChatGPTProvider::isAvailable() const
    {
        return _isAvailable;
    }
    std::string ChatGPTProvider::getModelName() const
    {
        return "gpt-4o-mini";
    }
    std::string ChatGPTProvider::getModelDesc() const
    {
        return "我是ChatGPT，由 OpenAI 开发的人工智能助手";
    }
    std::string ChatGPTProvider::sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam)
    {
        if(!isAvailable())
        {
            ERR("ChatGPTProvider is not available");
            return "";
        }

        double temperature = 0.7;
        int max_output_tokens = 2048;
        if(requestParam.find("temperature") != requestParam.end())
        {
            temperature = std::stod(requestParam.at("temperature"));
        }
        if(requestParam.find("max_output_tokens") != requestParam.end())
        {
            max_output_tokens = std::stoi(requestParam.at("max_output_tokens"));
        }

        Json::Value messageArray(Json::arrayValue);
        for(const auto& msg : messages)
        {
            Json::Value message;
            message["role"] = msg._role;
            message["content"] = msg._content;
            messageArray.append(message);
        }

        Json::Value request;
        request["model"] = getModelName();
        request["input"] = messageArray;
        request["temperature"] = temperature;
        request["max_output_tokens"] = max_output_tokens;

        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "";
        std::string requestBodyStr = Json::writeString(writerBuilder, request);
        INFO("ChatGPTProvider sendMessage requestBody: {}", requestBodyStr);
        //创建http客户端
        httplib::Client client(_endPoint.c_str());
        client.set_connection_timeout(30.0);
        client.set_read_timeout(60.0);

        //创建请求头
        httplib::Headers headers{
            {"Authorization", "Bearer " + _apiKey},
            //{"Content-Type", "application/json"}
        };

        //发起Post请求
        auto  response = client.Post("/v1/responses" , headers , requestBodyStr , "application/json");
        INFO("ChatGPTProvider sendMessage POST request success, status : {}", response->status);
        INFO("ChatGPTProvider sendMessage POST request success, body : {}", response->body);

        //解析模型的响应结果
        if(response->status == 200)
        {
            //反序列化
            std::istringstream responseStream(response->body);
            Json::Value responseBody;
            Json::CharReaderBuilder readerBuilder;
            std::string parseError;
            if(!Json::parseFromStream(readerBuilder, responseStream , &responseBody , &parseError))
            {
                ERR("parse json failed: {}" , parseError.c_str());
                return "";
            }
            //检测text字段是否存在，如果存在就返回
            if(responseBody.isMember("output") && responseBody["output"].isArray() && !responseBody["output"].empty())
            {
                // 模型的回复刚好是output数组的第0个元素
                auto output = responseBody["output"][0];
                if(output.isMember("content") && output["content"].isArray() && !output["content"].empty() && output["content"][0].isMember("text"))
                {
                    std::string replyString = output["content"][0]["text"].asString();
                    INFO("ChatGPTProvider sendMessage replyString: {}", replyString);
                    return replyString;
                }
            }
        }

        return "";
    }
    std::string ChatGPTProvider::sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , std::function<void(const std::string& , bool)> callback)
        {
           if(!isAvailable())
            {
                ERR("ChatGPTProvider is not available");
                return "";
            }

            double temperature = 0.7;
            int max_output_tokens = 2048;
            if(requestParam.find("temperature") != requestParam.end())
            {
                temperature = std::stod(requestParam.at("temperature"));
            }
            if(requestParam.find("max_output_tokens") != requestParam.end())
            {
                max_output_tokens = std::stoi(requestParam.at("max_output_tokens"));
            }

             Json::Value messageArray(Json::arrayValue);
            for(const auto& msg : messages)
            {
                Json::Value message;
                message["role"] = msg._role;
                message["content"] = msg._content;
                messageArray.append(message);
            }

            Json::Value request;
            request["model"] = getModelName();
            request["input"] = messageArray;
            request["temperature"] = temperature;
            request["max_output_tokens"] = max_output_tokens;
            request["stream"] = true;

             //对请求体进行序列化
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            std::string requestBodyStr = Json::writeString(writerBuilder, request);
            INFO("DeepSeekProvider sendMessageStream requestBody: {}", requestBodyStr);

            //创建http客户端
            httplib::Client client(_endPoint.c_str());
            client.set_connection_timeout(30.0);
            client.set_read_timeout(300.0); //流式响应需要设置超时时间较长
            //创建请求头
            httplib::Headers headers{
                {"Authorization", "Bearer " + _apiKey},
                {"Content-Type", "application/json"},
                {"Accept", "text/event-stream"}
            };

            //流式处理变量
            std::string buffer; // 接收流式响应的数据块
            bool getError = false;
            std::string errorMsg;
            int statusCode = 0;
            bool streamFinish = false; //标记流式响应是否结束
            std::string fullResponse; // 得到解析后的完整的消息

            //创建并配置请求对象
            httplib::Request req;
            req.method = "POST";
            req.path = "/v1/responses";
            req.body = requestBodyStr;
            req.headers = headers;

            //设置响应处理器
            req.response_handler = [&](const httplib::Response& response)->bool
            {
                if(response.status != 200)
                {
                    ERR("sendMessageStream failed, status code: {}" , response.status);
                    getError = true;
                    errorMsg = "HTTP status code: " + std::to_string(response.status);
                    return false;
                }
                return true;
            };

            req.content_receiver = [&](const char* data , size_t data_length , size_t offset , size_t total_length)->bool
                {
                    if(getError)
                    {
                        ERR("sendMessageStream failed, error: {}" , errorMsg.c_str());
                        return false;
                    }

                    buffer += std::string(data , data_length);
                    std::string eventType;
                    std::string eventData;
                    size_t pos = 0;
                    while((pos = buffer.find("\n\n")) != std::string::npos)
                    {
                        std::string chunk  = buffer.substr(0 , pos);
                        buffer.erase(0 , pos + 2);
                        if(chunk.empty() || chunk[0] == ':')
                        continue;

                        // 按 \n 分割 chunk，逐行解析 event: 和 data: 
                        {
                            std::istringstream lineStream(chunk);
                            std::string line;
                            while(std::getline(lineStream, line))
                            {
                                if(line.compare(0 , 6 ,"event:") == 0)
                                {
                                    eventType = line.substr(7);
                                }
                                else if(line.compare(0 , 6 ,"data: ") == 0)
                                {
                                    eventData = line.substr(6);
                                }
                            }
                        }

                        std::istringstream chunkDataStream(eventData);
                        Json::Value eventDataJson;
                        Json::CharReaderBuilder readerBuilder;
                        std::string errors;
                        if(!Json::parseFromStream(readerBuilder, chunkDataStream , &eventDataJson , &errors))
                        {
                            ERR("DeepSeekProvider sendMessageStream parse eventData error: {}" , errors.c_str());
                            continue;
                        }

                        if(eventType == "response.output_text.delta")
                        {
                            if(eventDataJson.isMember("delta") && eventDataJson["delta"].isString())
                            {
                                std::string delta = eventDataJson["delta"].asString();
                                callback(delta, false);
                            }
                        }
                        else if(eventType == "response.output_item.done")
                        {
                            if(eventDataJson.isMember("item") && eventDataJson["item"].isObject() &&\
                               eventDataJson["item"].isMember("content") && eventDataJson["item"]["content"].isArray()&&\
                               !eventDataJson["item"]["content"].empty() && eventDataJson["item"]["content"][0].isMember("text")&&\
                               eventDataJson["item"]["content"][0]["text"].isString())
                            {
                                fullResponse += eventDataJson["item"]["content"][0]["text"].asString();
                            }
                        }
                        else if(eventType == "response.completed")
                        {
                            streamFinish = true;
                            callback("", true);
                            return true;
                        }
                    }
                    return true;
                };

                //发起请求
                httplib::Response res;
                bool connect_ok = client.send(req , res);
                if(!connect_ok)
                {
                    ERR("sendMessageStream failed");
                    return "";
                }

                if(!streamFinish)
                {
                    callback("" , true);
                }
                return fullResponse;
        }
}