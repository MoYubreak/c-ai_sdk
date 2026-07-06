#pragma once
#include<string>
#include<vector>

namespace ai_chat_sdk
{
    struct Message
    {
        std::string _messageId;
        std::string _role;
        std::string _content;
        std::time_t _timestamp;
        Message(const std::string& role = "", const std::string& content = "")
        :_role(role) , _content(content)
        {}
    };

    struct Config
    {
        std::string _modelName;
        double _temperature;
        int _maxTokens;
    };

    struct APIConfig:public Config
    {
        std::string _apiKey;
    };

    struct ModelInfo
    {
        std::string _modelName;
        std::string _modelDesc; //模型描述
        std::string _provider; //模型提供方
        std::string _endpoint; //模型API的base url
        bool _isAvailable; //模型是否可用
        ModelInfo(const std::string& modelName = "", const std::string& modelDesc = "", const std::string& provider = "", const std::string& endpoint = "" , bool isAvailable = false)
        :_modelName(modelName) , _modelDesc(modelDesc) , _provider(provider) , _endpoint(endpoint) , _isAvailable(isAvailable)
        {}
    };

    struct Session //会话结构
    {
        std::string _sessionId;
        std::string _modelName;
        std::vector<Message> _messages;
        std::time_t _createAt;
        std::time_t _updateAt;
        Session(const std::string& modelName = "")
        :_modelName(modelName)
        {}
    };
}