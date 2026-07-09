#include"../include/ChatSDK.h"
#include"../include/DeepSeekProvider.h"
#include"../include/GeminiProvider.h"
#include"../include/ChatGPTProvider.h"
#include"../include/OllamaLLMProvider.h"
#include"../include/until/mylog.h"
#include <unordered_set>
#include <iomanip>

namespace ai_chat_sdk
{
        bool ChatSDK::initModels(const std::vector<std::shared_ptr<Config>>& configs)
        {
            INFO("ChatSDK::initModels starting...");
            registerAllProvider(configs);
            INFO("ChatSDK::initModels registerAllProvider done");
            initProviders(configs);
            INFO("ChatSDK::initModels initProviders done");
            _initialized = true;
            INFO("ChatSDK::initModels complete");
            return true;
        }
        std::string ChatSDK::createSession(const std::string& modelName)
        {
            if(!_initialized)
            {
                return "";
            }
            return _sessionManager.createSession(modelName);
        }
        std::shared_ptr<Session> ChatSDK::getSession(const std::string& sessionId)
        {
            if(!_initialized)
            {
                return nullptr;
            }
            return _sessionManager.getSession(sessionId);
        }
        std::vector<std::string> ChatSDK::getSessionLists() const
        {
            if(!_initialized)
            {
                return {};
            }
            return _sessionManager.getSessionLists();
        }
        bool ChatSDK::deleteSession(const std::string& sessionId)
        {
            if(!_initialized)
            {
                return false;
            }
            return _sessionManager.deleteSession(sessionId);
        }

        std::vector<Message> ChatSDK::getHistroyMessages(const std::string& sessionId)
        {
            if(!_initialized)
            {
                return {};
            }
            return _sessionManager.getHistroyMessages(sessionId);
        }


        std::string ChatSDK::sendMessage(const std::string& sessionId,const std::string& message)
        {
            if(!_initialized)
            {
                return "";
            }
            auto session = _sessionManager.getSession(sessionId);
            Message msg;
            msg._role = "user";
            msg._content = message;
            _sessionManager.addMessage(sessionId , msg);
            
            auto historyMessages = _sessionManager.getHistroyMessages(sessionId);

            std::map<std::string , std::string> requestParam;
            if(_modelConfigs.find(session->_modelName) != _modelConfigs.end())
            {
                auto config = _modelConfigs[session->_modelName];
                requestParam["temperature"] = std::to_string(config->_temperature);
                requestParam["maxTokens"] = std::to_string(config->_maxTokens);
            }
            else
            {
                ERR("ChatSDK::sendMessage: model config not found");
                return "";
            }
            auto response = _llmManager.sendMessage(session->_modelName , historyMessages , requestParam);
            Message respMsg;
            respMsg._role = "assistant";
            respMsg._content = response;
            _sessionManager.addMessage(sessionId , respMsg);
            _sessionManager.updateSessionTimestamp(sessionId);
            return response;

        }
        std::string ChatSDK::sendMessageStream(const std::string& sessionId , const std::string& message ,\
            std::function<void(const std::string& , bool)> callback)
            {
                if(!_initialized)
                {
                    return "";
                }
                auto session = _sessionManager.getSession(sessionId);
                Message msg;
                msg._role = "user";
                msg._content = message;
                _sessionManager.addMessage(sessionId , msg);
                
                auto historyMessages = _sessionManager.getHistroyMessages(sessionId);

                std::map<std::string , std::string> requestParam;
                if(_modelConfigs.find(session->_modelName) != _modelConfigs.end())
                {
                    auto config = _modelConfigs[session->_modelName];
                    requestParam["temperature"] = std::to_string(config->_temperature);
                    requestParam["maxTokens"] = std::to_string(config->_maxTokens);
                }
                else
                {
                    ERR("ChatSDK::sendMessageStream: model config not found");
                    return "";
                }
                auto response = _llmManager.sendMessageStream(session->_modelName , historyMessages , requestParam , callback);
                Message respMsg;
                respMsg._role = "assistant";
                respMsg._content = response;
                _sessionManager.addMessage(sessionId , respMsg);
                _sessionManager.updateSessionTimestamp(sessionId);
                return response;
            }

        bool ChatSDK::registerAllProvider(const std::vector<std::shared_ptr<Config>>& configs)
        {
            INFO("ChatSDK::registerAllProvider starting...");
            if(!_llmManager.isModelAvailable("deepseek-v4-flash"))
            {
                INFO("ChatSDK::registerAllProvider creating DeepSeekProvider...");
                auto deepseek = std::make_unique<DeepSeekProvider>();
                INFO("ChatSDK::registerAllProvider registering DeepSeekProvider...");
                _llmManager.registerProvider("deepseek-v4-flash" , std::move(deepseek));
            }

            if(!_llmManager.isModelAvailable("gemini-2.5-flash"))
            {
                INFO("ChatSDK::registerAllProvider creating GeminiProvider...");
                auto gemini = std::make_unique<GeminiProvider>();
                INFO("ChatSDK::registerAllProvider registering GeminiProvider...");
                _llmManager.registerProvider("gemini-2.5-flash" , std::move(gemini));
            }
            
            if(!_llmManager.isModelAvailable("gpt-4o-mini"))
            {
                INFO("ChatSDK::registerAllProvider creating ChatGPTProvider...");
                auto gpt = std::make_unique<ChatGPTProvider>();
                INFO("ChatSDK::registerAllProvider registering ChatGPTProvider...");
                _llmManager.registerProvider("gpt-4o-mini" , std::move(gpt));
            }

            std::unordered_set<std::string> configName;
            for(const auto& config : configs)
            {
                auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(config);
                if(ollamaConfig)
                {
                    if(configName.find(ollamaConfig->_modelName) != configName.end())
                    {
                        continue;
                    }
                    configName.insert(ollamaConfig->_modelName);
                    INFO("ChatSDK::registerAllProvider creating OllamaLLMProvider for {}", ollamaConfig->_modelName);
                    auto ollama = std::make_unique<OllamaLLMProvider>();
                    _llmManager.registerProvider(ollamaConfig->_modelName , std::move(ollama));
                }
            }
            INFO("ChatSDK::registerAllProvider complete");
            return true;
        }
        void ChatSDK::initProviders(const std::vector<std::shared_ptr<Config>>& configs)
        {
            for(const auto& config : configs)
            {
                auto apiConfig = std::dynamic_pointer_cast<APIConfig>(config);
                if(apiConfig)
                {
                    if(apiConfig->_modelName == "deepseek-v4-flash" || \
                    apiConfig->_modelName == "gemini-2.5-flash" || \
                    apiConfig->_modelName == "gpt-4o-mini")
                    {
                        initAPIModelProviders(apiConfig->_modelName , apiConfig);
                        continue;
                    }
                }
                auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(config);
                if(ollamaConfig)
                {
                    initOllamaModelProviders(ollamaConfig->_modelName , ollamaConfig);
                    continue;
                }
            }
        }

        bool ChatSDK::initAPIModelProviders(const std::string modelName , const std::shared_ptr<APIConfig>& apiConfig)
        {
            if(modelName.empty())
            {
                ERR("ChatSDK::initAPIModelProviders: modelName is empty");
                return false;
            }
            if(!apiConfig || apiConfig->_apiKey.empty())
            {
                ERR("ChatSDK::initAPIModelProviders: apiConfig or apiKey is empty");
                return false;
            }

            if(_llmManager.isModelAvailable(modelName))
            {
                ERR("ChatSDK::initAPIModelProviders: model {} is already available" , modelName);
                return false;
            }

            std::map<std::string , std::string> modelParam;
            modelParam["api_Key"] = apiConfig->_apiKey;
            
            if(!_llmManager.initModel(modelName , modelParam))
            {
                ERR("ChatSDK::initAPIModelProviders: initModel {} failed" , modelName);
                return false;
            }
            
            _modelConfigs[modelName] = apiConfig;
            
            return true;
        }
        
        bool ChatSDK::initOllamaModelProviders(const std::string modelName ,const std::shared_ptr<OllamaConfig>& ollamaConfig)
        {
            if(modelName.empty())
            {
                ERR("ChatSDK::initOllamaModelProviders: modelName is empty");
                return false;
            }
            if(!ollamaConfig || ollamaConfig->_endpoint.empty())
            {
                ERR("ChatSDK::initOllamaModelProviders: ollamaConfig or endpoint is empty");
                return false;
            }

            if(_llmManager.isModelAvailable(modelName))
            {
                ERR("ChatSDK::initOllamaModelProviders: model {} is already available" , modelName);
                return false;
            }
            
            std::map<std::string , std::string> modelParam;
            modelParam["base_URL"] = ollamaConfig->_endpoint;
            modelParam["model_name"] = ollamaConfig->_modelName;
            modelParam["model_desc"] = ollamaConfig->_modelDesc;
            
            if(!_llmManager.initModel(modelName , modelParam))
            {
                ERR("ChatSDK::initOllamaModelProviders: initModel {} failed" , modelName);
                return false;
            }
            
            _modelConfigs[modelName] = ollamaConfig;
            
            return true;
        }


}
