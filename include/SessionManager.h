#pragma once 
#include<string>
#include<map>
#include<unordered_map>
#include<functional>
#include<vector>
#include<memory>
#include<atomic>
#include<mutex>
#include"common.h"

namespace ai_chat_sdk
{
    class SessionManager
    {
    public:
        std::string createSession(const std::string& modelName);
        std::shared_ptr<Session> getSession(const std::string& sessionId);
        bool addMessage(const std::string& sessionId,const Message& message) const;
        std::vector<Message> getHistroyMessages(const std::string& sessionId);
        void updateSessionTimestamp(const std::string& sessionId);
        std::vector<std::string> getSessionLists() const;
        bool deleteSession(const std::string& sessionId);
        void clearAllSessions();
        size_t getSessionCount(const std::string& sessionId);
    private:
        std::string generateSessionId();
        std::string generateMessageId(std::size_t messageCounter);
    private:
        std::unordered_map<std::string , std::shared_ptr<Session>> _sessions;
        mutable std::mutex _mutex;
        std::atomic<int64_t> _sessionCounter;
    }
}