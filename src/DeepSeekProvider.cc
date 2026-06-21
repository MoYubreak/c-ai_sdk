#include"../include/DeepSeekProvider.h"
#include"../include/until/mylog.h"
#include<jsoncpp/json/json.h>
#include<sstream>

namespace ai_chat_sdk
{
   
        bool DeepSeekProvider::initModel(const std::map<std::string , std::string>& modelConfig)
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
            if(baseUrl == modelConfig.end())
            {
                ERR("base_URL is not found in modelConfig");
                return false;
            }
            _endPoint = baseUrl->second;

            _isAvailable = true;
            return _isAvailable;
        }
        bool DeepSeekProvider::isAvailable() const
        {
            return _isAvailable;
        }
        std::string DeepSeekProvider::getModelName() const
        {
            return "deepseek-v4-flash";
        }
        std::string DeepSeekProvider::getModelDesc() const
        {
            return "一款实用性强，中文优化的ai助手";
        }
        std::string DeepSeekProvider::sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam)
        {
            if(isAvailable())
            {
                double temperature = 0.7;
                int max_tokens = 2048;
                if(requestParam.find("temperature") != requestParam.end())
                {
                    temperature = std::stod(requestParam.at("temperature"));
                }
                if(requestParam.find("max_tokens") != requestParam.end())
                {
                    max_tokens = std::stoi(requestParam.at("max_tokens"));
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
                request["messages"] = messageArray;
                request["temperature"] = temperature;
                request["max_tokens"] = max_tokens;

                //序列化
                Json::StreamWriterBuilder writerBuilder;
                writerBuilder["indentation"] = "";
                std::string requestBodyStr = Json::writeString(writerBuilder, request);
                INFO("DeepSeekProvider sendMessage requestBody: {}", requestBodyStr);
                //创建http客户端
                httplib::Client client(_endPoint.c_str());
                client.set_connection_timeout(30.0);
                client.set_read_timeout(60.0);

                //创建请求头
                httplib::Headers headers{
                    {"Authorization", "Bearer " + _apiKey},
                    {"Content-Type", "application/json"}
                };

                //发起Post请求
                auto  response = client.Post("/chat/completions" , headers , requestBodyStr , "application/json");
                INFO("DeepSeekProvider sendMessage POST request success, status : {}", response->status);
                INFO("DeepSeekProvider sendMessage POST request success, body : {}", response->body);
                //解析模型的响应结果
                if(response->status == 200)
                {
                    std::istringstream responseStream(response->body);
                    Json::Value responseBody;
                    Json::CharReaderBuilder readerBuilder;
                    std::string parseError;
                    if(!Json::parseFromStream(readerBuilder, responseStream , &responseBody , &parseError))
                    {
                        ERR("parse json failed: {}" , parseError.c_str());
                        return "";
                    }
                    //检测content字段是否存在，如果存在就返回
                    if(responseBody.isMember("choices") &&responseBody["choices"].isArray() && !responseBody["choices"].empty())
                    {
                        auto choice = responseBody["choices"][0];
                        if(choice.isMember("message") && choice["message"].isMember("content"))
                        {
                            std::string replyContent = choice["message"]["content"].asString();
                            INFO("DeepSeekProvider response text: {}", replyContent);
                            return replyContent;
                        }
                    }
                }
            }
            ERR("sendMessage failed");
            return "";
        }
        std::string DeepSeekProvider::sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            const std::function<void(const std::string& , bool)> callback)
            {
                if(!isAvailable())
                {
                    ERR("sendMessageStream failed, model is not available");
                    return "";
                }

                //构建请求参数
                double temperature = 0.7;
                int max_tokens = 2048;
                if(requestParam.find("temperature") != requestParam.end())
                {
                    temperature = std::stod(requestParam.at("temperature"));
                }
                if(requestParam.find("max_tokens") != requestParam.end())
                {
                    max_tokens = std::stoi(requestParam.at("max_tokens"));
                }
                //构建历史消息
                Json::Value messageArray(Json::arrayValue);
                for(const auto& msg : messages)
                {
                    Json::Value message;
                    message["role"] = msg._role;
                    message["content"] = msg._content;
                    messageArray.append(message);
                }

                //构建请求体
                Json::Value request;
                request["model"] = getModelName();
                request["messages"] = messageArray;
                request["temperature"] = temperature;
                request["max_tokens"] = max_tokens;
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
                bool goError = false;
                std::string errorMsg;
                int statusCode = 0;
                bool streamFinish = false; //标记流式响应是否结束
                std::string fullResponse; // 得到解析后的完整的消息

                //创建并配置请求对象
                httplib::Request req;
                req.method = "POST";
                req.path = "/chat/completions";
                req.body = requestBodyStr;
                req.headers = headers;

                //设置响应处理器
                req.response_handler = [&](const httplib::Response& response)->bool
                {
                    if(response.status != 200)
                    {
                        ERR("sendMessageStream failed, status code: {}" , response.status);
                        goError = true;
                        errorMsg = "HTTP status code: " + std::to_string(response.status);
                        return false;
                    }
                    return true;
                };

                //设置数据接收处理器
                req.content_receiver = [&](const char* data , size_t data_length , size_t offset , size_t total_length)->bool
                {
                    if(goError)
                    {
                        return false;
                    }

                    //数据添加到buffer
                    buffer += std::string(data , data_length);
                    
                    //分割data，来拿到有效消息，并反序列化 发送给callback调用
                    size_t pos = 0;
                    while((pos = buffer.find("\n\n")) != std::string::npos)
                    {
                        std::string chunk  = buffer.substr( 0 , pos);
                        buffer.erase(0 , pos + 2);

                        if(chunk.empty() || chunk[0] == ':')
                            continue;

                        if(chunk.compare(0 , 6 , "data: ") == 0)
                        {
                            std::string chunkData = chunk.substr(6);
                            //判断流式响应是否完成
                            if(chunkData == "[DONE]")
                            {
                                streamFinish = true;
                                return true;
                            }

                            //对数据块的有效数据进行反序列化
                            std::istringstream chunkDataStream(chunkData);
                            Json::Value chunkDataJson;
                            Json::CharReaderBuilder readerBuilder;
                            std::string errors;
                            if(!Json::parseFromStream(readerBuilder, chunkDataStream , &chunkDataJson , &errors))
                            {
                                ERR("DeepSeekProvider sendMessageStream parse modelDataJson error: {}" , errors.c_str());
                            }
                            else
                            {
                                // 反序列化后 把 content 添加到 fullResponse 中，并调用 用户的callback来对数据进行处理
                                if(chunkDataJson.isMember("choices") && chunkDataJson["choices"].isArray() && !chunkDataJson["choices"].empty()\
                                    && chunkDataJson["choices"][0].isMember("delta") && chunkDataJson["choices"][0]["delta"].isMember("content"))
                                {
                                    std::string content = chunkDataJson["choices"][0]["delta"]["content"].asString();
                                    fullResponse += content;
                                     // 将本次解析出的模型返回的有效数据转给调用sendMessageStream函数的用户使用---callback
                                    callback(content , false);
                                }
                            }

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
        
