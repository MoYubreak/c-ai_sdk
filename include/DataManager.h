#include<sqlite3.h>
#include<string>
#include<mutex>
#include<memory>
#include<time>
#include<vector>
#include<iostream>
#include"common.h"

namespace ai_chat_sdk
{
    class DataManager
    {
    public:
        DataManager(const std::string& dbName);
        ~DataManager();

        bool insertSession(const Session& session);
        std::shared_ptr<Session> getSession(const std::string& sessionId) const;
        bool updateSessionTimestamp(const std::string& sessionId , std::time_t timestamp);
        bool deleteSession(const std::string& sessionId);
        std::vector<std::string> getAllSessionIds() const;
        std::vector<std::shared_ptr<Session>> getAllSessions() const;
        int getSessionCount() const;
        void clearAllSessions();

        bool insertMessage(const std::string& sessionId , const Message& message);
        std::vector<Message> getSessionMessage(const std::string& sessionId)const;
        bool deleteSessionMessage(const std::string& sessionId);
        
    private:
        bool initDataBase();
        bool execSQL(const std::string& sql);
    private:
        sqlite3* _db;
        std::string _dbName;
        mutable std::mutex _mutex;
    };
}