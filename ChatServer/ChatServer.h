#pragma once
#include <httplib.h>
#include <memory>
#include <ai_chat_sdk/ChatSDK.h>

namespace ai_chat_server
{
    struct ServerConfig
    {
        std::string ip;
        int port;

        double temperature;
        int maxTokens;

        std::string deepseekAPIKey;
        std::string chatgptAPIKey;
        std::string geminiAPIKey;

        std::string ollamaModelName;
        std::string ollamaModelDesc;
        std::string ollamaEndpoint;
    };
    class ChatServer
    {
    public:
        ChatServer(const ServerConfig& config);

        void start();
        void stop();
        bool isRunning() const;

    private:
        //构造错误响应
        std::string createResponse(const std::string& message, bool sucess);
        //处理创建会话请求
        std::string handleCreateSessionRequest(const httplib::Request& request , httplib::Response& response);
        //处理获取会话列表请求
        std::string handleGetSessionListsRequest(const httplib::Request& request , httplib::Response& response);
        //处理获取模型列表请求
        std::string handleGetModelListsRequest(const httplib::Request& request , httplib::Response& response);
        //处理获取历史消息请求
        std::string handleGetHistroyMessagesRequest(const httplib::Request& request , httplib::Response& response);
        //处理删除会话请求
        std::string handleDeleteSessionRequest(const httplib::Request& request , httplib::Response& response);
        //处理发送消息请求---全量返回
        std::string handleSendMessageRequest(const httplib::Request& request , httplib::Response& response);
        //处理发送消息流请求---流式返回
        std::string handleSendMessageStreamRequest(const httplib::Request& request , httplib::Response& response);

        //路由规则绑定
        void setHttpRoutes();


    private:
        std::shared_ptr<ai_chat_sdk::ChatSDK> _chatSDK = nullptr;
       std::unique_ptr<httplib::Server> _chatServer = nullptr;
       std::atomic<bool> _isRunning = {false};
       ServerConfig _config;
    };
}
