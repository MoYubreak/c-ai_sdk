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

                Json::Value messageArray;
                for(auto& msg : messages)
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

            }

}