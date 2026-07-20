#pragma once
#include"SessionManager.h"
#include"LLMManager.h"
#include"DataManager.h"

namespace ai_chat_sdk
{
    class ChatSDK
    {
    private:
        bool registerAllProvider(const std::vector<std::shared_ptr<Config>>& configs);
        void initProviders(const std::vector<std::shared_ptr<Config>>& configs);

        bool initAPIModelProviders(const std::string modelName , const std::shared_ptr<APIConfig>& apiConfig);
        bool initOllamaModelProviders(const std::string modelName , const std::shared_ptr<OllamaConfig>& ollamaConfig);
    public:
        bool initModels(const std::vector<std::shared_ptr<Config>>& configs);
        std::string createSession(const std::string& modelName);
        std::shared_ptr<Session> getSession(const std::string& sessionId);
        bool deleteSession(const std::string& sessionId);
        std::vector<std::string> getSessionLists() const;

        std::vector<Message> getHistroyMessages(const std::string& sessionId);
        std::vector<ModelInfo> getAvailableModels()const;


        std::string sendMessage(const std::string& sessionId,const std::string& message);
        std::string sendMessageStream(const std::string& sessionId , const std::string& message ,\
            std::function<void(const std::string& , bool)> callback);
    private:
        SessionManager _sessionManager;
        LLMManager  _llmManager;
        std::unordered_map<std::string , std::shared_ptr<Config>> _modelConfigs;
        bool _initialized;
    };
}
