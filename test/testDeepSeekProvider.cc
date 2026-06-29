#include"../include/DeepSeekProvider.h"
#include"../include/ChatGPTProvider.h"
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


TEST(ChatGPTProviderTest , sendMessage)
{
    auto chatgpt = std::make_shared<ai_chat_sdk::ChatGPTProvider>();
    ASSERT_TRUE(chatgpt != nullptr);

    std::map<std::string , std::string> modelParam;
    modelParam["api_Key"] = ::getenv("chatgpt_apikey");
    modelParam["base_URL"] = "https://api.openai.com";

    chatgpt->initModel(modelParam);

    std::vector<ai_chat_sdk::Message> messages;
    messages.push_back(ai_chat_sdk::Message("user" , "你好"));
    std::map<std::string , std::string> requestParam;
    requestParam["temperature"] = "0.7";
    requestParam["max_output_tokens"] = "2048";


    std::string response = chatgpt->sendMessage(messages , requestParam);
    ASSERT_FALSE(response.empty());

    INFO("response: {}" , response);
}

int main(int argc , char** argv)
{
    //初始化googleTest
    testing::InitGoogleTest(&argc , argv);
    //初始化日志Logger
    //ns_logger::Logger::initLogger("testDeepSeekProvider.log" , "stdout");
    ns_logger::Logger::initLogger("testChatGPTProvider.log" , "stdout");

    //执行所有测试用例并返回
    return RUN_ALL_TESTS();
}
