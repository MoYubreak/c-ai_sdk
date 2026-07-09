#pragma once 
#include<string>
#include<map>
#include<functional>
#include<vector>
#include<memory>
#include"httplib.h"
#include"common.h"
#include"LLMProvider.h"

namespace ai_chat_sdk
{
    class LLMManager
    {
    public:
        LLMManager();
        bool registerProvider(const std::string& modelName , std::unique_ptr<LLMProvider> provider);
        bool initModel(const std::string& modelName , const std::map<std::string , std::string>& modelParam);
        std::vector<ModelInfo> getAvailableModels() const;
        bool isModelAvailable(const std::string& modelName) const;
        std::string sendMessage(const std::string& modelName , const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam);
        std::string sendMessageStream(const std::string& modelName , const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
            std::function<void(const std::string& , bool)> callback);
    private:
        std::map<std::string , std::unique_ptr<LLMProvider>> _providers;
        std::map<std::string , ModelInfo> _modelInfos;
    };
}