#include"../include/DeepSeekProvider.h"
#include"../include/ChatGPTProvider.h"
#include"../include/GeminiProvider.h"
#include"../include/OllamaLLMProvider.h"
#include"../include/ChatSDK.h"
#include<gtest/gtest.h>
#include<memory>

// TEST(DeepSeekProviderTest , sendMessage)
// {
//     auto deepseek = std::make_shared<ai_chat_sdk::DeepSeekProvider>();
//     ASSERT_TRUE(deepseek != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("deepseek_apikey");
//     modelParam["base_URL"] = "https://api.deepseek.com";

//     deepseek->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你好"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_tokens"] = "2048";


//     std::string response = deepseek->sendMessage(messages , requestParam);
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }

// TEST(DeepSeekProviderTest , sendMessage)
// {
//     auto deepseek = std::make_shared<ai_chat_sdk::DeepSeekProvider>();
//     ASSERT_TRUE(deepseek != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("deepseek_apikey");
//     modelParam["base_URL"] = "https://api.deepseek.com";

//     deepseek->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你好"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_tokens"] = "2048";


//     std::string response = deepseek->sendMessageStream(messages , requestParam , [](const std::string& chunk , bool isFinish)
//     {
//         if(!isFinish)
//         {
//             INFO("response: {}" , chunk);
//         }
//         else 
//         {
//             INFO("[DONE]");
//         }
//     });
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }


// TEST(ChatGPTProviderTest , sendMessage)
// {
//     auto chatgpt = std::make_shared<ai_chat_sdk::ChatGPTProvider>();
//     ASSERT_TRUE(chatgpt != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("chatgpt_apikey");
//     modelParam["base_URL"] = "https://api.openai.com";

//     chatgpt->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你好"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_output_tokens"] = "2048";


//     std::string response = chatgpt->sendMessage(messages , requestParam);
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }

// TEST(ChatGPTProviderTest , sendMessageStream)
// {
//     auto chatgpt = std::make_shared<ai_chat_sdk::ChatGPTProvider>();
//     ASSERT_TRUE(chatgpt != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("chatgpt_apikey");
//     modelParam["base_URL"] = "https://api.openai.com";  

//     chatgpt->initModel(modelParam);  
//     messages.push_back(ai_chat_sdk::Message("user" , "你好"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_output_tokens"] = "2048";


//     std::string response = chatgpt->sendMessageStream(messages , requestParam , [](const std::string& chunk , bool isFinish)
//     {
//         if(!isFinish)
//         {
//             INFO("response: {}" , chunk);
//         }
//         else 
//         {
//             INFO("[DONE]");
//         }
//     });
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }

// TEST(GeminiProviderTest , sendMessage)
// {
//     auto gemini = std::make_shared<ai_chat_sdk::GeminiProvider>();
//     ASSERT_TRUE(gemini != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("gemini_apikey");
//     modelParam["base_URL"] = "https://generativelanguage.googleapis.com";

//     gemini->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你是谁"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_tokens"] = "2048";


//     std::string response = gemini->sendMessage(messages , requestParam);
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }


// TEST(GeminiProviderTest , sendMessageStream)
// {
//     auto gemini = std::make_shared<ai_chat_sdk::GeminiProvider>();
//     ASSERT_TRUE(gemini != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["api_Key"] = ::getenv("gemini_apikey");
//     modelParam["base_URL"] = "https://generativelanguage.googleapis.com";

//     gemini->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你是谁"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_tokens"] = "2048";


//     std::string response = gemini->sendMessageStream(messages , requestParam , [](const std::string& chunk , bool isFinish)
//     {
//         if(!isFinish)
//         {
//             INFO("response: {}" , chunk);
//         }
//         else 
//         {
//             INFO("[DONE]");
//         }
//     });
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }


// TEST(OllamaLLMProviderTest , sendMessageStream)
// {
//     auto ollama = std::make_shared<ai_chat_sdk::OllamaLLMProvider>();
//     ASSERT_TRUE(ollama != nullptr);

//     std::map<std::string , std::string> modelParam;
//     modelParam["model_name"] = "deepseek-r1:1.5b";
//     modelParam["model_desc"] = "本地部署的deepseek-r1:1.5b模型，采用专家混合架构，专注于深度理解和推理";
//     modelParam["base_URL"] = "http://localhost:11434";

//     ollama->initModel(modelParam);

//     std::vector<ai_chat_sdk::Message> messages;
//     messages.push_back(ai_chat_sdk::Message("user" , "你好"));
//     std::map<std::string , std::string> requestParam;
//     requestParam["temperature"] = "0.7";
//     requestParam["max_tokens"] = "2048";


//     // std::string response = ollama->sendMessage(messages , requestParam);
//     // ASSERT_FALSE(response.empty());

//     // INFO("response: {}" , response);


//     std::string response = ollama->sendMessageStream(messages , requestParam , [](const std::string& chunk , bool isFinish)
//     {
//         if(!isFinish)
//         {
//             INFO("response: {}" , chunk);
//         }
//         else 
//         {
//             INFO("[DONE]");
//         }
//     });
//     ASSERT_FALSE(response.empty());

//     INFO("response: {}" , response);
// }

TEST(ChatSDKTest , sendMessage)
{
    auto sdk = std::make_shared<ai_chat_sdk::ChatSDK>();
    ASSERT_TRUE(sdk != nullptr);

    auto deepseekConfig = std::make_shared<ai_chat_sdk::APIConfig>();
    ASSERT_TRUE(deepseekConfig != nullptr);
    deepseekConfig->_apiKey = ::getenv("deepseek_apikey");
    deepseekConfig->_modelName = "deepseek-v4-flash";
    ASSERT_FALSE(deepseekConfig->_apiKey.empty());
    deepseekConfig->_temperature = 0.7;
    deepseekConfig->_maxTokens = 2048;
    INFO("deepseekConfig: {}" , deepseekConfig->_modelName);

    auto gptConfig = std::make_shared<ai_chat_sdk::APIConfig>();
    ASSERT_TRUE(gptConfig != nullptr);
    gptConfig->_apiKey = ::getenv("chatgpt_apikey");
    gptConfig->_modelName = "gpt-4o-mini";
    ASSERT_FALSE(gptConfig->_apiKey.empty());
    gptConfig->_temperature = 0.7;
    gptConfig->_maxTokens = 2048;
    INFO("gptConfig: {}" , gptConfig->_modelName);

    auto geminiConfig = std::make_shared<ai_chat_sdk::APIConfig>();
    ASSERT_TRUE(geminiConfig != nullptr);
    geminiConfig->_apiKey = ::getenv("gemini_apikey");
    geminiConfig->_modelName = "gemini-2.5-flash";
    ASSERT_FALSE(geminiConfig->_apiKey.empty());
    geminiConfig->_temperature = 0.7;
    geminiConfig->_maxTokens = 2048;
    INFO("geminiConfig: {}" , geminiConfig->_modelName);
    auto ollamaConfig = std::make_shared<ai_chat_sdk::OllamaConfig>();
    ASSERT_TRUE(ollamaConfig != nullptr);
    ollamaConfig->_modelName = "deepseek-r1:1.5b";
    ollamaConfig->_modelDesc = "本地部署的deepseek-r1:1.5b模型，采用专家混合架构，专注于深度理解和推理";
    ollamaConfig->_endpoint = "http://localhost:11434";
    ollamaConfig->_temperature = 0.7;
    ollamaConfig->_maxTokens = 2048;
    INFO("ollamaConfig: {}" , ollamaConfig->_modelName);
    std::vector<std::shared_ptr<ai_chat_sdk::Config>> configs = {deepseekConfig , gptConfig , geminiConfig , ollamaConfig};

    INFO("sdk->initModels begin");
    sdk->initModels(configs);
    INFO("sdk->initModels 成功");
    
    auto sessionId = sdk->createSession(deepseekConfig->_modelName);
    INFO("sdk->createSession 成功 , sessionId: {}" , sessionId);
    ASSERT_FALSE(sessionId.empty());
    std::string message;
    std::cout << ">>>";
    std::getline(std::cin , message);
    //auto response = sdk->sendMessage(sessionId , message);
    auto response = sdk->sendMessageStream(sessionId , message , [](const std::string& chunk , bool isFinish)
    {
        if(!isFinish)
        {
            INFO("response: {}" , chunk);
        }
        else 
        {
            INFO("[DONE]");
        }
    });
    ASSERT_FALSE(response.empty());
    INFO("response: {}" , response);

    std::cout << ">>>";
    std::getline(std::cin , message);
    //response = sdk->sendMessage(sessionId , message);
    response = sdk->sendMessageStream(sessionId , message , [](const std::string& chunk , bool isFinish)
    {
        if(!isFinish)
        {
            INFO("response: {}" , chunk);
        }
        else 
        {
            INFO("[DONE]");
        }
    });
    ASSERT_FALSE(response.empty());

    //获取会话历史消息
    auto messages = sdk->getHistroyMessages(sessionId);
    ASSERT_TRUE(!messages.empty());
    for(auto& msg : messages)
    {
        INFO("message: {}" , msg._content);
    }

}

int main(int argc , char** argv)
{
    //初始化googleTest
    testing::InitGoogleTest(&argc , argv);
    //初始化日志Logger
    //ns_logger::Logger::initLogger("testDeepSeekProvider.log" , "stdout");
    ns_logger::Logger::initLogger("testChatSDK.log" , "stdout");

    //执行所有测试用例并返回
    return RUN_ALL_TESTS();
}
