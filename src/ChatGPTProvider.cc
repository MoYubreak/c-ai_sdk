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
            if(baseUrl == modelConfig.end())
            {
                ERR("base_URL is not found in modelConfig");
                return false;
            }
            _endPoint = baseUrl->second;

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
            return "";
        }
}