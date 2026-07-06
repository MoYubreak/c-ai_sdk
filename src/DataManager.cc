#include"../include/DataManager.h"

namespace ai_chat_sdk
{
        DataManager::DataManager(const std::string& dbName)
        :_dbName(dbName)
        ,_db(nullptr)
        {
            int rc = sqlite3_open(dbName.c_str(), &_db);
            if(rc != 0)
            {
                ERR("DataManager::DataManager: sqlite3_open failed: {}", sqlite3_errmsg(_db));
                return;
            }

            if(!initDataBase())
            {
                sqlite3_close(_db);
                _db = nullptr;
            }
        }

        DataManager::~DataManager()
        {
            if(_db)
            {
                sqlite3_close(_db);
                _db = nullptr;
            }
        }


        bool DataManager::insertSession(const Session& session)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string insertSQL  = R"(
                INSERT INTO session (session_id, model_name, create_time, update_time)
                VALUES (?, ?, ?, ?);
            )";

            sqlite3_stmt* stmt = nullptr;
            int rc = sqlite3_prepare_v2(_db, insertSQL.c_str(), -1, &stmt, nullptr);
            if(rc != 0)
            {
                ERR("DataManager::insertSession: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                return false;
            }
            sqlite3_bind_text(stmt, 1, session.sessionId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, session.modelName.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 3, static_cast<int64_t>(session._createdAt));
            sqlite3_bind_int64(stmt, 4, static_cast<int64_t>(session._updatedAt));
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::insertSession: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            
            sqlite3_finalize(stmt);
            INFO("insertSession - 插入会话成功：{}", session._sessionId);
            return true;
        }
        std::shared_ptr<Session> DataManager::getSession(const std::string& sessionId) const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string selectSQL = R"(
                SELECT session_id, model_name, create_time, update_time
                FROM session
                WHERE session_id = ?
            )";

            sqlite3_stmt* stmt = nullptr;
            int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                return nullptr;
            }

            // 从结果集中提取数据
            std::string modelName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int64_t createTime = sqlite3_column_int64(stmt, 1);
            int64_t updateTime = sqlite3_column_int64(stmt, 2);


            Session session;
            session.sessionId = sessionId;
            session.modelName = modelName;
            session._createdAt = createTime;
            session._updatedAt = updateTime;
            sqlite3_finalize(stmt);

            session.messages = getSessionMessage(sessionId);
            return std::make_shared<Session>(session);
        }

        bool DataManager::updateSessionTimestamp(const std::string& sessionId , std::time_t timestamp)
        {
            std::lock_guard<std::mutex> lock(_mutex);

            std::string updateSQL = R"(
                UPDATE session
                SET update_time = ?
                WHERE session_id = ?
            )";

            sqlite3_stmt* stmt = nullptr;
            int rc = sqlite3_prepare_v2(_db, updateSQL.c_str(), -1, &stmt, nullptr);

            sqlite3_bind_int64(stmt, 1, static_cast<int64_t>(timestamp));
            sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_STATIC);


            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::updateSessionTimestamp: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }

            sqlite3_finalize(stmt);
            INFO("updateSessionTimestamp - 更新会话时间戳成功：{}", sessionId);
            return true;
        }
        bool DataManager::deleteSession(const std::string& sessionId)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string deleteSQL = R"(
                DELETE FROM session
                WHERE session_id = ?
            )";

            sqlite3_stmt* stmt = nullptr;
            int rc = sqlite3_prepare_v2(_db, deleteSQL.c_str(), -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::deleteSession: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }

            sqlite3_finalize(stmt);
            INFO("deleteSession - 删除会话成功：{}", sessionId);
            return true;
        }
        std::vector<std::string> DataManager::getAllSessionIds() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string selectSQL = R"(
                SELECT session_id FROM session ORDER BY update_time DESC
            )";

            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::getAllSessionIds: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return {};
            }

            std::vector<std::string> sessionIds;
            while(rc = sqlite3_step(stmt) == SQLITE_ROW)
            {
                sessionIds.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            }

            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::getAllSessionIds: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return {};
            }
            sqlite3_finalize(stmt);
            INFO("getAllSessionIds - 获取所有会话ID成功，共话数：{}", sessionIds.size());
            return sessionIds;

        }
        std::vector<std::shared_ptr<Session>> DataManager::getAllSessions() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string selectSQL = R"(
                SELECT session_id, model_name, create_time, update_time FROM session ORDER BY update_time DESC
            )";

            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::getAllSessions: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return {};
            }

            std::vector<std::shared_ptr<Session>> sessions;
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                std::string sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string modelName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                int64_t createTime = sqlite3_column_int64(stmt, 2);
                int64_t updateTime = sqlite3_column_int64(stmt, 3);
                Session session;
                session.sessionId = sessionId;
                session.modelName = modelName;
                session._createAt  = createTime;
                session._updateAt = updateTime;
                sessions.push_back(std::make_shared<Session>(session));
            }
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::getAllSessions: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return {};
            }
            sqlite3_finalize(stmt);
            INFO("getAllSessions - 获取所有会话成功，共话数：{}", sessions.size());
            return sessions;
        }
        int DataManager::getSessionCount() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string selectSQL = R"(
                SELECT COUNT(*) FROM session ORDER BY update_time DESC
            )";

            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::getSessionCount: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return 0;
            }
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::getSessionCount: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return 0;
            }
            int count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            INFO("getSessionCount - 获取会话数量成功：{}", count);
            return count;
        }

        bool DataManager::insertMessage(const std::string& sessionId , const Message& message) 
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string insertSQL = R"(
                INSERT INTO message (message_id, role, content, timestamp)
                VALUES (?, ?, ?, ?)
            )";

            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, insertSQL.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::insertMessage: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                return false;
            }
            sqlite3_bind_text(stmt, 1, message._messageId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, message._role.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, message._content.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 4, static_cast<int64_t>(message._timestamp));

            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::insertMessage: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }


            sqlite3_stmt* SessionStmt;
            rc = sqlite3_prepare_v2(_db, R"(
                UPDATE session SET update_time = ? WHERE session_id = ?
            )", -1, &SessionStmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::insertMessage: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            sqlite3_bind_int64(SessionStmt, 1, static_cast<int64_t>(message._timestamp));
            sqlite3_bind_text(SessionStmt, 2, sessionId.c_str(), -1, SQLITE_STATIC);
            rc = sqlite3_step(SessionStmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::insertMessage: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                sqlite3_finalize(SessionStmt);
                return false;
            }

            sqlite3_finalize(SessionStmt);
            sqlite3_finalize(stmt);
            INFO("insertMessage - 插入消息成功，会话ID：{}，消息ID：{}", sessionId, message._messageId);
            return true;
        }
        
        std::vector<Message> DataManager::getSessionMessage(const std::string& sessionId)const;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string sql = R"(
                SELECT message_id, role, content, timestamp FROM message WHERE session_id = ?;
            )";
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::getSessionMessage: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                return {};
            }
            std::vector<Message> messages;
            while(rc = sqlite3_step(stmt) == SQLITE_ROW)
            {
                Message message;
                message._messageId = sqlite3_column_text(stmt, 0);
                message._role = sqlite3_column_text(stmt, 1);
                message._content = sqlite3_column_text(stmt, 2);
                message._timestamp = sqlite3_column_int64(stmt, 3);
                messages.push_back(message);
            }

            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::getSessionMessage: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return {};
            }
            
            sqlite3_finalize(stmt);
            return messages;
        }
        bool DataManager::deleteSessionMessage(const std::string& sessionId)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string sql = R"(
                DELETE FROM message WHERE session_id = ?;
            )";
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::deleteSessionMessage: sqlite3_prepare_v2 failed: {}", sqlite3_errmsg(_db));
                return false;
            }

            sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                ERR("DataManager::deleteSessionMessage: sqlite3_step failed: {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            sqlite3_finalize(stmt);
            return true;
        }

        bool DataManager::initDataBase()
        {
            std::string createSessionTable = R"(
                CREATE TABLE IF NOT EXISTS session (
                    session_id TEXT PRIMARY KEY,
                    model_name TEXT NOTNULL,
                    create_time INTEGER NOT NULL,
                    update_time INTEGER NOT NULL
                );
            )";
            if(!execSQL(createSessionTable))
            {
                return false;
            }

            std::string createMessageTable = R"(
                CREATE TABLE IF NOT EXISTS message (
                    message_id TEXT PRIMARY KEY,
                    session_id TEXT NOT NULL,
                    role TEXT NOT NULL,
                    content TEXT NOT NULL,
                    timestamp INTEGER NOT NULL,
                    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
                );
            )";
            if(!execSQL(createMessageTable))
            {
                return false;
            }
        }

        bool DataManager::execSQL(const std::string& sql)
        {
            if(!_db)
            {
                ERR("数据库未初始化");
                return false;
            }
            char* errMsg = nullptr;
            int rc = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg);
            if(rc != SQLITE_OK)
            {
                ERR("DataManager::execSQL: sqlite3_exec failed: {}", errMsg);
                sqlite3_free(errMsg);
                return false;
            }
            return true;
        }
}
