#include"../include/OllamaLLMProvider.h"
#include"../include/until/mylog.h"
#include<jsoncpp/json/json.h>
#include<sstream>

namespace ai_chat_sdk
{
        bool OllamaLLMProvider::initModel(const std::map<std::string , std::string>& modelConfig)
        {
            auto model_name = modelConfig.find("model_name");
            if(model_name == modelConfig.end())
            {
                ERR("model_name is not found in modelConfig");
                return false;
            }
            _modelName = model_name->second;
            auto model_desc = modelConfig.find("model_desc");
            if(model_desc == modelConfig.end())
            {
                ERR("model_desc is not found in modelConfig");
                return false;
            }
            _modelDesc = model_desc->second;

            auto base_url = modelConfig.find("base_URL");
            if(base_url == modelConfig.end())
            {
                ERR("base_URL is not found in modelConfig");
                return false;
            }
            _endPoint = base_url->second;
            
            _isAvailable = true;
            return true;
        }
        bool OllamaLLMProvider::isAvailable() const
        {
            return _isAvailable;
        }
        std::string OllamaLLMProvider::getModelName() const
        {
            return _modelName;
        }
        std::string OllamaLLMProvider::getModelDesc() const
        {
            return _modelDesc;
        }
        std::string OllamaLLMProvider::sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam)
        {
            if(!isAvailable())
            {
                ERR("OllamaLLMProvider is not available");
                return "";
            }

            double temperature = 0.7;
            int max_tokens = 1024;
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

            Json::Value options;
            options["temperature"] = temperature;
            options["num_ctx"] = max_tokens;

            Json::Value requestBody;
            requestBody["model"] = _modelName;
            requestBody["messages"] = messageArray;
            requestBody["options"] = options;
            requestBody["stream"] = false;

            //序列化
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
            INFO("OllamaLLMProvider sendMessage requestBody: {}", requestBodyStr);
            //创建http客户端
            httplib::Client client(_endPoint.c_str());
            client.set_connection_timeout(30.0);
            client.set_read_timeout(60.0);

            //创建请求头
            httplib::Headers headers{
                {"Content-Type", "application/json"}
            };

            //发起Post请求
            auto  response = client.Post("/api/chat" , headers , requestBodyStr , "application/json");
            if(!response)
            {
                ERR("OllamaLLMProvider sendMessage POST request failed");
                return "";
            }
            INFO("OllamaLLMProvider sendMessage POST request success, status : {}", response->status);
            INFO("OllamaLLMProvider sendMessage POST request success, body : {}", response->body);

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
                //检测content字段是否存在，如果存在就返回
                // 检查错误字段（Ollama 可能返回）
                if (responseBody.isMember("error"))
                {
                    ERR("Ollama returned error: {}", responseBody["error"].asString());
                    return "";
                }

                // 正确提取 message.content
                if (responseBody.isMember("message") && responseBody["message"].isMember("content"))
                {
                    std::string replyContent = responseBody["message"]["content"].asString();
                    INFO("OllamaLLMProvider response text: {}", replyContent);
                    return replyContent;
                }
                else
                {
                    ERR("Response missing 'message.content' field");
                    return "";
                }
            }

            return "";
        }
        std::string OllamaLLMProvider::sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            std::function<void(const std::string& , bool)> callback)
        {
            if(!isAvailable())
            {
                ERR("OllamaLLMProvider is not available");
                return "";
            }

            double temperature = 0.7;
            int max_tokens = 1024;
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

            Json::Value options;
            options["temperature"] = temperature;
            options["num_ctx"] = max_tokens;

            Json::Value requestBody;
            requestBody["model"] = _modelName;
            requestBody["messages"] = messageArray;
            requestBody["options"] = options;
            requestBody["stream"] = true;

            //序列化
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
            INFO("OllamaLLMProvider sendMessage requestBody: {}", requestBodyStr);
            //创建http客户端
            httplib::Client client(_endPoint.c_str());
            client.set_connection_timeout(30.0);
            client.set_read_timeout(60.0);

            //创建请求头
            httplib::Headers headers{
                {"Content-Type", "application/json"}
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
            req.path = "/api/chat";
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

            //设置数据接收处理器
            req.content_receiver = [&](const char* data , size_t data_length , size_t offset , size_t total_length)->bool
            {
                if(getError)
                {
                    return false;
                }

                //数据添加到buffer
                buffer += std::string(data , data_length);
                
                //分割data，来拿到有效消息，并反序列化 发送给callback调用
                size_t pos = 0;
                while((pos = buffer.find("\n")) != std::string::npos)
                {
                    std::string chunk  = buffer.substr( 0 , pos);
                    buffer.erase(0 , pos + 1);

                    if(chunk.empty())
                        continue;

                
                    std::string chunkData = chunk;

                    //对数据块的有效数据进行反序列化
                    std::istringstream chunkDataStream(chunkData);
                    Json::Value chunkDataJson;
                    Json::CharReaderBuilder readerBuilder;
                    std::string errors;
                    if(!Json::parseFromStream(readerBuilder, chunkDataStream , &chunkDataJson , &errors))
                    {
                        ERR("OllamaLLMProvider sendMessageStream parse modelDataJson error: {}" , errors.c_str());
                    }

                    // 处理结束标记
                    if(chunkDataJson.get("done", false).asBool())
                    {
                        callback("", true);
                        streamFinish = true;
                        return true; 
                    }

                    if (chunkDataJson.isMember("message") && chunkDataJson["message"].isMember("content"))
                    {
                        std::string content = chunkDataJson["message"]["content"].asString();
                        fullResponse += content;
                        callback(content , false);
                    }

                    return true;
                };
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
                ERR("sendMessageStream failed, stream not finish");
                return "";
            }
            return fullResponse;
        }
}