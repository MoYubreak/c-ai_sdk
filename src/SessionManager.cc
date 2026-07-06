#include"../SessionManager.h"
#include<sstream>

namespace ai_chat_sdk
{
        SessionManager::SessionManager(const std::string dbName)
        : _dataManager(dbName)
        {
            _dataManager.initDataBase();
            auto sessions = _dataManager.getAllSessions();
            for(auto session : sessions)
            {
                _sessions[session->_sessionId] = session;
            }
        }

        std::string SessionManager::createSession(const std::string& modelName)
        {
            _mutex.lock();   
            std::lock_guard<std::mutex> lock(_mutex);
            std::string sessionId = generateSessionId();
            std::shared_ptr<Session> session = std::make_shared<Session>(modelName);
            session->_sessionId = sessionId;
            session->_createAt = time(nullptr);
            session->_updatedAt = session->_createdAt;


            _sessions[sessionId] = session;
            _mutex.unlock();

            _dataManager.insertSession(session);
            return sessionId;
        }
        std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it != _sessions.end())
            {
                _mutex.unlock();
                it->second->_messages = _dataManager.getMessagesBySessionId(sessionId);
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
        bool SessionManager::addMessage(const std::string& sessionId,const Message& message) const
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                _mutex.unlock();
                return false;
            }

            Message newMessage = message;
            newMessage._id = generateMessageId(it->second->_messages.size());
            it->second->_messages.push_back(newMessage);
            it->second->_updateAt = time(nullptr);
            _mutex.unlock();

            _dataManager.insertMessage(newMessage);
            return true;
        }
        std::vector<Message> SessionManager::getHistroyMessages(const std::string& sessionId)
        {
            _mutex.lock();
            auto it = _sessions.find(sessionId);
            if(it == _sessions.end())
            {
                _mutex.unlock();
                return {};
            }
            _mutex.unlock();
            auto messages = _dataManager.getSessionMessage(sessionId);
            return messages;
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
            std::vector<Message> messages = _dataManager.getAllMessages();

            _mutex.lock();
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

            for(auto message : messages)
            {
                if(sessionIds.find(message._sessionId) == sessionIds.end())
                {
                    sessionIds.push_back(message._sessionId);
                }
            }
            _mutex.unlock();
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
