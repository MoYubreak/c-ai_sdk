#include"../include/SessionManager.h"
#include<sstream>
#include<iomanip>
#include<atomic>

namespace ai_chat_sdk
{
        SessionManager::SessionManager(const std::string dbName)
        : _dataManager(dbName)
        {
            std::cout << "SessionManager constructor starting..." << std::endl;
            auto sessions = _dataManager.getAllSessions();
            std::cout << "SessionManager: getAllSessions done, got " << sessions.size() << " sessions" << std::endl;
            for(auto session : sessions)
            {
                _sessions[session->_sessionId] = session;
            }
            std::cout << "SessionManager constructor complete" << std::endl;
        }

        std::string SessionManager::createSession(const std::string& modelName)
        {
            _mutex.lock();   
            std::string sessionId = generateSessionId();
            std::shared_ptr<Session> session = std::make_shared<Session>(modelName);
            session->_sessionId = sessionId;
            session->_createAt = time(nullptr);
            session->_updateAt = session->_createAt;


            _sessions[sessionId] = session;
            _mutex.unlock();

            _dataManager.insertSession(*session);
            return sessionId;
        }
        std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it != _sessions.end())
            {
                _mutex.unlock();
                it->second->_messages = _dataManager.getSessionMessage(sessionId);
                return it->second;
            }
            _mutex.unlock();
            auto session = _dataManager.getSession(sessionId);
            _mutex.lock();
            if(_sessions.find(sessionId) == _sessions.end())
            {
                _sessions[sessionId] = session;
                _mutex.unlock();
                return session;
            }
            _mutex.unlock();
            return nullptr;
        }
        bool SessionManager::addMessage(const std::string& sessionId,const Message& message) 
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                _mutex.unlock();
                return false;
            }

            Message newMessage = message;
            newMessage._messageId = generateMessageId(it->second->_messages.size());
            it->second->_messages.push_back(newMessage);
            it->second->_updateAt = time(nullptr);
            _mutex.unlock();

            _dataManager.insertMessage(sessionId , newMessage);
            return true;
        }
        std::vector<Message> SessionManager::getHistroyMessages(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it != _sessions.end())
            {
                if(it->second->_messages.empty())
                {
                    // 从数据库加载到内存中
                    it->second->_messages = _dataManager.getSessionMessage(sessionId);
                }
                auto messages = it->second->_messages;
                _mutex.unlock();
                return messages;
            }
            _mutex.unlock();
            return _dataManager.getSessionMessage(sessionId);
        }
        void SessionManager::updateSessionTimestamp(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                _mutex.unlock();
                return;
            }
            it->second->_updateAt = time(nullptr);
            _mutex.unlock();
            _dataManager.updateSessionTimestamp(sessionId , it->second->_updateAt);
        }
        std::vector<std::string> SessionManager::getSessionLists() const
        {
            auto sessions = _dataManager.getAllSessions();
            _mutex.lock();
            std::vector<std::pair<std::time_t , std::shared_ptr<Session>>> temp;
            for(auto it = _sessions.begin() ; it != _sessions.end() ; it++)
            {
                temp.push_back({it->second->_updateAt , it->second});
            }
            for(auto session : sessions)
            {
                if(_sessions.find(session->_sessionId) == _sessions.end())
                {
                    temp.push_back({session->_updateAt , session});
                }
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


            _mutex.unlock();
            return sessionIds;

        }
        bool SessionManager::deleteSession(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                _mutex.unlock();
                return false;
            }
            _sessions.erase(it);
            _mutex.unlock();
            _dataManager.deleteSession(sessionId);
            return true;
        }
        void SessionManager::clearAllSessions()
        {
            _mutex.lock();
            _sessions.clear();
            _mutex.unlock();
            _dataManager.clearAllSessions();

        }
        size_t SessionManager::getSessionCount()
        {
            std::lock_guard<std::mutex> lock(_mutex);
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
