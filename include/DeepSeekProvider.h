#pragma once
#include"LLMProvider.h"
#include"./until/mylog.h"
#include<iostream>

namespace ai_chat_sdk
{
    class DeepSeekProvider: public LLMProvider
    {
    public:
        virtual bool initModel(const std::map<std::string , std::string>& modelConfig);
        virtual bool isAvailable() const;
        virtual std::string getModelName() const;
        virtual std::string getModelDesc() const;
        virtual std::string sendMessage(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam);
        virtual std::string sendMessageStream(const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            std::function<void(const std::string& , bool)> callback);
    };
}