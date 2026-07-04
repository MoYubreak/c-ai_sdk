#include"../include/DeepSeekProvider.h"
#include"../include/ChatGPTProvider.h"
#include"../include/GeminiProvider.h"
#include"../include/OllamaLLMProvider.h"
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


TEST(OllamaLLMProviderTest , sendMessageStream)
{
    auto ollama = std::make_shared<ai_chat_sdk::OllamaLLMProvider>();
    ASSERT_TRUE(ollama != nullptr);

    std::map<std::string , std::string> modelParam;
    modelParam["model_name"] = "deepseek-r1:1.5b";
    modelParam["model_desc"] = "本地部署的deepseek-r1:1.5b模型，采用专家混合架构，专注于深度理解和推理";
    modelParam["base_URL"] = "http://localhost:11434";

    ollama->initModel(modelParam);

    std::vector<ai_chat_sdk::Message> messages;
    messages.push_back(ai_chat_sdk::Message("user" , "你好"));
    std::map<std::string , std::string> requestParam;
    requestParam["temperature"] = "0.7";
    requestParam["max_tokens"] = "2048";


    // std::string response = ollama->sendMessage(messages , requestParam);
    // ASSERT_FALSE(response.empty());

    // INFO("response: {}" , response);


    std::string response = ollama->sendMessageStream(messages , requestParam , [](const std::string& chunk , bool isFinish)
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
}

int main(int argc , char** argv)
{
    //初始化googleTest
    testing::InitGoogleTest(&argc , argv);
    //初始化日志Logger
    //ns_logger::Logger::initLogger("testDeepSeekProvider.log" , "stdout");
    ns_logger::Logger::initLogger("testOllamaLLMProvider.log" , "stdout");

    //执行所有测试用例并返回
    return RUN_ALL_TESTS();
}
