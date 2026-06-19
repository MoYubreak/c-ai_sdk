#pragma once 
#include<string>
#include<map>
#include<function>
#include<vector>
#include"httplib.h"
#include"common.h"

namespace ai-chat-sdk
{
    class LLMProvider
    {
    public:
        virtual bool initModel(const std::map<std::string , std::string>& modelConfig) = 0;
        virtual bool isAvailable() const = 0;
        virtual std::string getModelName() const = 0;
        virtual std::string getModelDesc() const =0;
        virtual std::string sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam) = 0;
        virtual std::string sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            std::function<void(const std::string& , bool)> callback) = 0;
    protected:
        bool _isAvailable = false;
        std::string _apiKey;
        std::string _endPoint; //Base_URL
    };
}