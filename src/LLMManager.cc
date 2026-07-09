#include"../include/LLMManager.h"
#include"../include/until/mylog.h"
#include <iostream>

namespace ai_chat_sdk
{
    LLMManager::LLMManager()
    {
        std::cout << "LLMManager constructor starting..." << std::endl;
    }

    bool LLMManager::registerProvider(const std::string& modelName , std::unique_ptr<LLMProvider> provider)
    {
        if(!provider)
        {
            ERR("model {} is nullptr" , modelName);
            return false;
        }
        _providers[modelName] = std::move(provider);

        _modelInfos[modelName] = ModelInfo(modelName);
        return true;
    }
    bool LLMManager::initModel(const std::string& modelName , const std::map<std::string , std::string>& modelParam)
    {
        auto provider = _providers.find(modelName);
        if(provider == _providers.end())
        {
            ERR("model {} not registered" , modelName);
            return false;
        }
        provider->second->initModel(modelParam);
        _modelInfos[modelName]._isAvailable = true;
        _modelInfos[modelName]._modelDesc = provider->second->getModelDesc();
        return true;
    }
    std::vector<ModelInfo> LLMManager::getAvailableModels() const
    {
        std::vector<ModelInfo> models;
        for(auto& model : _modelInfos)
        {
            if(model.second._isAvailable)
            {
                models.push_back(model.second);
            }
        }
        return models;
    }
    bool LLMManager::isModelAvailable(const std::string& modelName) const
    {
        auto model = _modelInfos.find(modelName);
        if(model == _modelInfos.end())
        {
            INFO("LLMManager::isModelAvailable: model {} not registered, returning false" , modelName);
            return false;
        }
        INFO("LLMManager::isModelAvailable: model {}, available = {}" , modelName, model->second._isAvailable);
        return model->second._isAvailable;
    }
    std::string LLMManager::sendMessage(const std::string& modelName , const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam)
    {
        auto provider = _providers.find(modelName);
        if(provider == _providers.end())
        {
            ERR("model {} not registered" , modelName);
            return "";
        }
        
        if(!provider->second->isAvailable())
        {
            ERR("model {} is not available" , modelName);
            return "";
        }
        return provider->second->sendMessage(messages , requestParam);
    }
    std::string LLMManager::sendMessageStream(const std::string& modelName , const std::vector<Message>& messages , const std::map<std::string , std::string>& requestParam , \
        std::function<void(const std::string& , bool)> callback)
        {
            auto provider = _providers.find(modelName);
            if(provider == _providers.end())
            {
                ERR("model {} not registered" , modelName);
                return "";
            }
            if(!provider->second->isAvailable())
            {
                ERR("model {} is not available" , modelName);
                return "";
            }
            return provider->second->sendMessageStream(messages , requestParam , callback);
        }
}