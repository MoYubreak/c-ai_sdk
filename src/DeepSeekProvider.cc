#include"../include/DeepSeekProvider.h"
#include"../include/until/mylog.h"
#include<jsoncpp/json/json.h>
#include<istringstream>

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
            return "deepseek-v4-chat";
        }
        std::string DeepSeekProvider::getModelDesc() const
        {
            return "一款实用性强，中文优化的ai助手";
        }
        std::string DeepSeekProvider::sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam)
        {
            if(isAvailable())
            {
                double template = 0.7;
                int max_tokens = 2048;
                if(requestParam.find("template") != requestParam.end())
                {
                    template = std::stod(requestParam["template"]);
                }
                if(requestParam.find("max_tokens") != requestParam.end())
                {
                    max_tokens = std::stoi(requestParam["max_tokens"]);
                }

                Json::Value messageArray;
                for(auto& msg : messages)
                {
                    Json::Value message;
                    message["role"] = msg.role;
                    message["content"] = msg.content;
                    messageArray.append(message);
                }

                Json::Value request;
                request["model"] = getModelName();
                request["messages"] = messageArray;
                request["temperature"] = template;
                request["max_tokens"] = max_tokens;

                //序列化
                Json::StreamWriterBuilder writerBuilder;
                writerBuilder["indentation"] = "";
                std::string requestBodyStr = Json::writeString(writerBuilder, request);

                //创建http客户端
                httplib::Client client(_endPoint);
                client.SetContentTimeout(30.0);
                client.SetReadTimeout(60.0);

                //创建请求头
                httplib::Headers headers;
                headers["Authorization"] = "Bearer " + _apiKey;
                headers["Content-Type"] = "application/json";

                //发起Post请求
                httplib::Result response = client.Post("/chat/completions" , headers , requestBodyStr , "application/json");
                
                //解析模型的响应结果
                if(response.status == 200)
                {
                    std::istringstream responseStream(response.body);
                    Json::Value responseBody;
                    Json::CharReaderBuilder readerBuilder;
                    std::string parseError;
                    if(!Json::parseFromStream(readerBuilder, responseStream , responseBody , &parseError))
                    {
                        ERR("parse json failed: %s" , parseError.c_str());
                        return "";
                    }
                    //检测content字段是否存在，如果存在就返回
                    if(response.isMember("choices") &&response["choices"].isArray())
                    {
                        auto choice = response["choices"][0];
                        if(choice.isObject() && choice["message"].isObject()&&choice["message"]["content"].isString())
                        {
                            return choice["message"]["content"].asString();
                        }
                    }
                }
            }
            ERR("sendMessage failed");
            return "";
        }
        std::string DeepSeekProvider::sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            std::function<void(const std::string& , bool)> callback);

}