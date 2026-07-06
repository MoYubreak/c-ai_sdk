#include"../SessionManager.h"
#include<sstream>

namespace ai_chat_sdk
{
        std::string SessionManager::createSession(const std::string& modelName)
        {
            std::string sessionId = generateSessionId();
            std::shared_ptr<Session> session = std::make_shared<Session>(modelName);
            session->_sessionId = sessionId;
            _sessions[sessionId] = session;
            session->_createAt = time(nullptr);
            return sessionId;
        }
        std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId)
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                return nullptr;
            }
            return it->second;
        }
        bool SessionManager::addMessage(const std::string& sessionId,const Message& message) const
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                return false;
            }

            Message newMessage = message;
            newMessage._id = generateMessageId(it->second->_messages.size());
            it->second->_messages.push_back(newMessage);
            it->second->_updateAt = time(nullptr);
            return true;
        }
        std::vector<Message> SessionManager::getHistroyMessages(const std::string& sessionId)
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                return {};
            }
            return it->second->_messages;
        }
        void SessionManager::updateSessionTimestamp(const std::string& sessionId)
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                return;
            }
            it->second->_updateAt = time(nullptr);
        }
        std::vector<std::string> SessionManager::getSessionLists() const
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::pair<std::time_t , std::shared_ptr<Session>>> temp;
            for(auto it = _sessions.begin() ; it != _sessions.end() ; it++)
            {
                temp.push_back({it->second->_updateAt , it->second});
            }
            std::sort(temp.begin() , temp.end() , [](const std::pair<std::time_t , std::shared_ptr<Session>>& a , const std::pair<std::time_t , std::shared_ptr<Session>>& b)
            {
                return a.first > b.first;
            });
            std::vector<std::string> sessionIds;
            for(auto it = temp.begin() ; it != temp.end() ; it++)
            {
                sessionIds.push_back(it->second->_sessionId);
            }
            return sessionIds;

        }
        bool SessionManager::deleteSession(const std::string& sessionId)
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                return false;
            }
            _sessions.erase(it);
            return true;
        }
        void SessionManager::clearAllSessions()
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            _sessions.clear();
        }
        size_t SessionManager::getSessionCount()
        {
            mutex::lock_guard<std::mutex> lock(_mutex);
            return _sessions.size();
        }

        std::string SessionManager::generateSessionId()
        {
            _sessionCounter.fetch_add(1);

            time_t tm = time(nullptr);
            
            std::stringstream ss;
            ss << "session" << "_" << tm << std::setw(8) << std::setfill('0') << _sessionCounter.load();
            return ss.str();
        }
        std::string SessionManager::generateMessageId(std::size_t messageCounter)
        {
            messageCounter++;
            time_t tm = time(nullptr);
            
            std::stringstream ss;
            ss << "message" << "_" << tm << std::setw(8) << std::setfill('0') << messageCounter;
            return ss.str();
        }
}
