#include"ChatServer.h"
#include<jsoncpp/json/json.h>
#include<fstream>

namespace ai_chat_server
{
    ChatServer::ChatServer(const ServerConfig& config)
    {
        
        auto deepseekConfig = std::make_shared<ai_chat_sdk::APIConfig>();
        deepseekConfig->_apiKey = config.deepseekAPIKey;
        deepseekConfig->_modelName = "deepseek-v4-flash";
        deepseekConfig->_temperature = config.temperature;
        deepseekConfig->_maxTokens = config.maxTokens;
        INFO("deepseekConfig: {}" , deepseekConfig->_modelName);

        auto gptConfig = std::make_shared<ai_chat_sdk::APIConfig>();
        gptConfig->_apiKey = config.chatgptAPIKey;
        gptConfig->_modelName = "gpt-4o-mini";
        gptConfig->_temperature = config.temperature;
        gptConfig->_maxTokens = config.maxTokens;
        INFO("gptConfig: {}" , gptConfig->_modelName);

        auto geminiConfig = std::make_shared<ai_chat_sdk::APIConfig>();
        geminiConfig->_apiKey = config.geminiAPIKey;
        geminiConfig->_modelName = "gemini-2.5-flash";
        geminiConfig->_temperature = config.temperature;
        geminiConfig->_maxTokens = config.maxTokens;
        INFO("geminiConfig: {}" , geminiConfig->_modelName);
        auto ollamaConfig = std::make_shared<ai_chat_sdk::OllamaConfig>();
        ollamaConfig->_modelName = config.ollamaModelName;
        ollamaConfig->_modelDesc = config.ollamaModelDesc;
        ollamaConfig->_endpoint = config.ollamaEndpoint;
        ollamaConfig->_temperature = config.temperature;
        ollamaConfig->_maxTokens = config.maxTokens;
        INFO("ollamaConfig: {}" , ollamaConfig->_modelName);
        std::vector<std::shared_ptr<ai_chat_sdk::Config>> configs = {deepseekConfig , gptConfig , geminiConfig , ollamaConfig};

        _chatSDK = std::make_shared<ai_chat_sdk::ChatSDK>();

        if(_chatSDK->initModels(configs))
        {
            INFO("initModels success");
        }
        else
        {
            CRI("initModels failed");
        }
        
        //初始化_server
        _chatServer = std::make_unique<httplib::Server>();
        
        //初始化_config
        _config = config;
    }

    void ChatServer::start()
    {
        if(_isRunning.load())
        {
            ERR("ChatServer::start: server is running");
            return;
        }
        INFO("setHttpRoutes begin");
        setHttpRoutes();
        INFO("setHttpRoutes end");
        
        // 尝试挂载 www 目录用于静态文件服务
        // 先尝试多个可能的相对路径，以适应不同的启动目录
        bool mountSuccess = _chatServer->set_mount_point("/" , "./www");
        if (!mountSuccess)
        {
            mountSuccess = _chatServer->set_mount_point("/" , "../www");
            if (!mountSuccess)
            {
                mountSuccess = _chatServer->set_mount_point("/" , "build/www");
                if (!mountSuccess)
                {
                    WARN("无法挂载 www 目录，请确保启动目录包含 www 子目录");
                }
            }
        }
        
        // 添加根路径处理，确保即使挂载失败也能访问首页
        _chatServer->Get("/", [this](const httplib::Request& req, httplib::Response& res) {
            // 先检查 ./www/index.html
            std::ifstream file("./www/index.html");
            if (!file.is_open())
            {
                file.open("../www/index.html");
            }
            if (!file.is_open())
            {
                file.open("build/www/index.html");
            }
            
            if (file.is_open())
            {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                res.set_content(content, "text/html; charset=utf-8");
            }
            else
            {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
            }
        });
        
        std::thread serverThread([this]()
        {
            _chatServer->listen(_config.ip.c_str() , _config.port);
        });
        serverThread.detach();

        _isRunning.store(true);
    }
    
    void ChatServer::stop()
    {
        if(!_isRunning.load())
        {
            ERR("ChatServer::stop: server is not running");
            return;
        }
        _chatServer->stop();
        _isRunning.store(false);
    }

    bool ChatServer::isRunning() const
    {
        return _isRunning.load();
    }

    std::string ChatServer::createResponse(const std::string& message , bool sucess)
    {
        Json::Value errResponse;
        errResponse["success"] = sucess;
        errResponse["message"] = message;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        return Json::writeString(builder , errResponse);
    }
    //处理创建会话请求
    void ChatServer::handleCreateSessionRequest(const httplib::Request& request , httplib::Response& response)
    {
        Json::Reader reader;        
        Json::Value requestJson;
        if(!reader.parse(request.body , requestJson))
        {
            std::string errResponse = createResponse("parse request body failed" , false);
            response.status =400;
            response.set_content(errResponse , "application/json");
            return;
        }

        std::string modelName = requestJson.get("model" , "deepseek-chat").asString();

        std::string sessionId = _chatSDK->createSession(modelName);
        if(sessionId.empty())
        {
            std::string errResponse = createResponse("create session failed" , false);
            response.status =500;
            response.set_content(errResponse , "application/json");
            return;
        }

        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "create session success";
        Json::Value data;
        data["session_id"] = sessionId;
        data["model"] = modelName;
        responseJson["data"] = data;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string responseJsonStr = Json::writeString(builder, responseJson);
        response.status = 200;
        response.set_content(responseJsonStr , "application/json");
    }
    //处理获取会话列表请求
    void ChatServer::handleGetSessionListsRequest(const httplib::Request& request , httplib::Response& response)
    {
        std::vector<std::string> sessionLists = _chatSDK->getSessionLists();
        Json::Value dataArray(Json::arrayValue);
        for(auto sessionId : sessionLists)
        {
            auto session = _chatSDK->getSession(sessionId);
            if(session != nullptr)
            {
                Json::Value data;
                data["id"] = sessionId;
                data["model"] = session->_modelName;
                data["created_at"] = session->_createAt;
                data["updated_at"] = session->_updateAt;
                data["message_count"] = session->_messages.size();
                if(!session->_messages.empty())
                    data["first_user_message"] = session->_messages.front()._content;
                dataArray.append(data);
            }
        }


        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get session lists success";
        responseJson["data"] = dataArray;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string responseJsonStr = Json::writeString(builder, responseJson);
        response.status = 200;
        response.set_content(responseJsonStr , "application/json");
    }
    //处理获取模型列表请求
    void ChatServer::handleGetModelListsRequest(const httplib::Request& request , httplib::Response& response)
    {
        std::vector<ai_chat_sdk::ModelInfo> modelLists = _chatSDK->getAvailableModels();
        Json::Value dataArray(Json::arrayValue);
        for(auto modelInfo : modelLists)
        {
            Json::Value data;
            data["model"] = modelInfo._modelName;
            data["desc"] = modelInfo._modelDesc;
            dataArray.append(data);
        }

        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get model lists success";
        responseJson["data"] = dataArray;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string responseJsonStr = Json::writeString(builder, responseJson);
        response.status = 200;
        response.set_content(responseJsonStr , "application/json");
    }
    //处理获取历史消息请求
    void ChatServer::handleGetHistroyMessagesRequest(const httplib::Request& request , httplib::Response& response)
    {
        std::string sessionId = request.matches[1];
        auto session = _chatSDK->getSession(sessionId);
        if(session == nullptr)
        {
            std::string errResponse = createResponse("session not found" , false);
            response.status =404;
            response.set_content(errResponse , "application/json");
            return;
        }
        std::vector<ai_chat_sdk::Message> messages = session->_messages;
        
        Json::Value dataArray(Json::arrayValue);
        for(auto message : messages)
        {
            Json::Value data;
            data["id"] = message._messageId;
            data["role"] = message._role;
            data["content"] = message._content;
            data["timestamp"] = static_cast<int64_t>(message._timestamp);
            dataArray.append(data);
        }

        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get histroy messages success";
        responseJson["data"] = dataArray;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string responseJsonStr = Json::writeString(builder, responseJson);
        response.status = 200;
        response.set_content(responseJsonStr , "application/json");
    }
    //处理删除会话请求
    void ChatServer::handleDeleteSessionRequest(const httplib::Request& request , httplib::Response& response)
    {
        std::string sessionId = request.matches[1];
        if(sessionId.empty())
        {
            std::string errResponse = createResponse("session not found" , false);
            response.status =404;
            response.set_content(errResponse , "application/json");
            return;
        }
        

        
        if(! _chatSDK->deleteSession(sessionId))
        {
            std::string errResponse = createResponse("delete session failed" , false);
            response.status =500;
            response.set_content(errResponse , "application/json");
        }
        else
        {
            std::string responseSuccess = createResponse("delete session success" , true);
            response.status = 200;
            response.set_content(responseSuccess , "application/json");
        }
    }
    //处理发送消息请求---全量返回
    void ChatServer::handleSendMessageRequest(const httplib::Request& request , httplib::Response& response)
    {
        Json::Reader reader;        
        Json::Value requestJson;
        if(!reader.parse(request.body , requestJson))
        {
            std::string errResponse = createResponse("parse request body failed" , false);

            response.status =400;
            response.set_content(errResponse , "application/json");
            return;
        }

        std::string sessionId = requestJson["session_id"].asString();
        std::string message = requestJson["message"].asString();
        if(sessionId.empty() || message.empty())
        {
            std::string errResponse = createResponse("session_id or message is empty" , false);

            response.status =400;
            response.set_content(errResponse , "application/json");
            return;
        }

        std::string result = _chatSDK->sendMessage(sessionId , message);
        if(result.empty())
        {
            std::string errResponse = createResponse("send message failed" , false);
            response.status =500;
            response.set_content(errResponse , "application/json");
            return;
        }

        Json::Value data;
        data["session_id"] = sessionId;
        data["response"] = result;

        Json::Value responseData;
        responseData["data"] = data;
        responseData["success"] = true;
        responseData["message"] = "send message success";

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string responseJsonStr = Json::writeString(builder, responseData);
        response.status = 200;
        response.set_content(responseJsonStr , "application/json");
    }
    //处理发送消息流请求---流式返回
    void ChatServer::handleSendMessageStreamRequest(const httplib::Request& request , httplib::Response& response)
    {
        Json::Reader reader;        
        Json::Value requestJson;
        if(!reader.parse(request.body , requestJson))
        {
            std::string errResponse = createResponse("parse request body failed" , false);

            response.status =400;
            response.set_content(errResponse , "application/json");
            return;
        }

        std::string sessionId = requestJson["session_id"].asString();
        std::string message = requestJson["message"].asString();
        if(sessionId.empty() || message.empty())
        {
            std::string errResponse = createResponse("session_id or message is empty" , false);

            response.status =400;
            response.set_content(errResponse , "application/json");
            return;
        }

        response.set_header("Cache-Control" , "no-cache");
        response.set_header("Connection" , "keep-alive");
        response.set_header("Access-Control-Allow-Origin", "*");        // 允许跨域请求
        response.set_header("Access-Control-Allow-Headers", "*");      // 允许所有请求头

        response.set_chunked_content_provider("text/event-stream" , [this , request , response , sessionId , message](size_t offset , httplib::DataSink& dataSink){

            // 防止 httplib 多次调用 provider（offset > 0 表示重入）
            if (offset > 0) {
                dataSink.done();
                return false;
            }

            auto writeChunk  = [&dataSink](const std::string& chunk , bool isFinish)
            {
                std::string sseData = "data: " + Json::valueToQuotedString(chunk.c_str()) + "\n\n";

                dataSink.write(sseData.c_str(), sseData.size());

                if(isFinish)
                {
                    std::string doneData = "data: [DONE]\n\n";
                    dataSink.write(doneData.c_str(), doneData.size());
                    dataSink.done();    
                    return false; 
                }

                return true;
            };

            if (!writeChunk("", false)) {
            return false;
            }

            std::string result = _chatSDK->sendMessageStream(sessionId , message , writeChunk );

            return true;
        });
    }


    void ChatServer::setHttpRoutes()
    {
        // 处理创建会话请求
        _chatServer->Post("/api/session", [this](const httplib::Request& request, httplib::Response& response){
            handleCreateSessionRequest(request, response);
        });

        // 处理获取会话列表请求
        _chatServer->Get("/api/sessions", [this](const httplib::Request& request, httplib::Response& response){
            handleGetSessionListsRequest(request, response);
        }); 

        // 处理获取模型列表请求
        _chatServer->Get("/api/models", [this](const httplib::Request& request, httplib::Response& response){
            INFO("进入Get模型列表方法");
            handleGetModelListsRequest(request, response);
            INFO("退出Get模型列表方法");
        });

        // 处理删除会话请求
        _chatServer->Delete("/api/session/(.*)", [this](const httplib::Request& request, httplib::Response& response){
            handleDeleteSessionRequest(request, response);
        });

        // 处理获取历史消息请求
        _chatServer->Get("/api/session/(.*)/history", [this](const httplib::Request& request, httplib::Response& response){
            handleGetHistroyMessagesRequest(request, response);
        });



        // 处理发送消息请求-全量返回
        _chatServer->Post("/api/message", [this](const httplib::Request& request, httplib::Response& response){
            handleSendMessageRequest(request, response);
        });

        // 处理发送消息请求-增量返回
        _chatServer->Post("/api/message/async", [this](const httplib::Request& request, httplib::Response& response){
            handleSendMessageStreamRequest(request, response);
        });
    }
}
