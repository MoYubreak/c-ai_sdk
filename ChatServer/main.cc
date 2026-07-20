#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <csignal>
#include <string>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include "ChatServer.h"

// ============================================================
// gflags 定义 ServerConfig 参数
// ============================================================

DEFINE_string(ip, "0.0.0.0",
    "服务器绑定的 IP 地址 (默认: 0.0.0.0)");

DEFINE_int32(port, 8080,
    "服务器监听的端口 (默认: 8080)");

DEFINE_string(log_level, "INFO",
    "日志级别，可选: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL (默认: INFO)");

DEFINE_double(temperature, 0.7,
    "模型生成温度，取值范围 [0.0, 2.0] (默认: 0.7)");

DEFINE_int32(max_tokens, 2048,
    "最大生成 token 数，不能为负数 (默认: 2048)");

DEFINE_string(ollama_model_name, "deepseek-r1:1.5b",
    "Ollama 本地模型名称 (如: deepseek-r1:1.5b)");

DEFINE_string(ollama_model_desc, "Local DeepSeek Model",
    "Ollama 本地模型描述");

DEFINE_string(ollama_endpoint, "http://localhost:11434",
    "Ollama 服务端点地址 (如: http://localhost:11434)");

// ============================================================
// 版本号
// ============================================================
static const char* kVersion = "1.0.0";

// ============================================================
// 帮助信息
// ============================================================
static void PrintHelpMessage()
{
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                    AIChatServer v)";
    std::cout << kVersion << std::endl;
    std::cout << R"(║
╠══════════════════════════════════════════════════════════════╣
║  一个支持多模型（DeepSeek、ChatGPT、Gemini、Ollama）的      ║
║  AI 聊天服务端程序。                                        ║
╚══════════════════════════════════════════════════════════════╝

■ 参数说明:

  --ip=<address>          服务器绑定的 IP 地址 (默认: 0.0.0.0)
  --port=<port>           服务器监听的端口号 (默认: 8080)
  --log_level=<level>     日志级别: TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL (默认: INFO)
  --temperature=<value>   模型生成温度，范围 [0.0, 2.0] (默认: 0.7)
  --max_tokens=<number>   最大生成 token 数 (默认: 2048)
  --ollama_model_name      Ollama 本地模型名称 (默认: deepseek-r1:1.5b)
  --ollama_model_desc      Ollama 本地模型描述 (默认: Local DeepSeek Model)
  --ollama_endpoint        Ollama 服务端点地址 (默认: http://localhost:11434)
  --flagfile=<path>       从配置文件加载参数 (如: --flagfile=ChatServer.conf)

  API Key 通过环境变量设置（无需命令行参数）:
    deepseek_apikey      DeepSeek API 密钥
    chatgpt_apikey       ChatGPT API 密钥
    gemini_apikey        Gemini API 密钥

■ 使用示例:

  1. 使用默认配置启动:
     ./AIChatServer

  2. 指定 IP 和端口:
     ./AIChatServer --ip=0.0.0.0 --port=9090

  3. 从配置文件加载:
     ./AIChatServer --flagfile=ChatServer.conf

  4. 查看此帮助:
     ./AIChatServer -h
     ./AIChatServer --help

  5. 查看版本号:
     ./AIChatServer -v
     ./AIChatServer --version

  6. 完整示例（配置 API Key + Ollama）:
     export deepseek_apikey=your_key_here
     export chatgpt_apikey=your_key_here
     export gemini_apikey=your_key_here
     ./AIChatServer --ip=0.0.0.0 --port=8080        \
       --ollama_model_name=llama3                    \
       --ollama_model_desc="Local Llama3 Model"      \
       --ollama_endpoint=http://localhost:11434      \
       --temperature=0.8 --max_tokens=4096

■ ChatServer 提供的 API 接口:

  POST   /api/session                   创建新会话
  GET    /api/sessions                  获取会话列表
  GET    /api/models                    获取可用模型列表
  GET    /api/session/{session_id}/history  获取会话历史消息
  DELETE /api/session/{session_id}      删除指定会话
  POST   /api/message                   发送消息（全量返回）
  POST   /api/message/async             发送消息（流式返回）

  启动后可在浏览器访问: http://<ip>:<port>
  或通过 curl / Postman 等工具调用上述 API。

)" << std::endl;
}

// ============================================================
// 版本信息
// ============================================================
static void PrintVersion()
{
    std::cout << "AIChatServer version " << kVersion << std::endl;
}

// ============================================================
// 日志级别转换
// ============================================================
static spdlog::level::level_enum StringToLogLevel(const std::string& level)
{
    std::string upper;
    for (char c : level)
        upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));

    if (upper == "TRACE")    return spdlog::level::trace;
    if (upper == "DEBUG")    return spdlog::level::debug;
    if (upper == "INFO")     return spdlog::level::info;
    if (upper == "WARN")     return spdlog::level::warn;
    if (upper == "ERROR")    return spdlog::level::err;
    if (upper == "CRITICAL") return spdlog::level::critical;

    std::cerr << "[WARNING] 未知的日志级别: " << level
              << "，使用默认级别 INFO" << std::endl;
    return spdlog::level::info;
}

// ============================================================
// 参数安全检测
// ============================================================
static bool ValidateConfig()
{
    bool valid = true;

    // 温度值在 0~2 之间
    if (FLAGS_temperature < 0.0 || FLAGS_temperature > 2.0)
    {
        std::cerr << "[ERROR] temperature 必须在 [0.0, 2.0] 范围内，当前值: "
                  << FLAGS_temperature << std::endl;
        valid = false;
    }

    // 最大 token 数不能为负数
    if (FLAGS_max_tokens < 0)
    {
        std::cerr << "[ERROR] max_tokens 不能为负数，当前值: "
                  << FLAGS_max_tokens << std::endl;
        valid = false;
    }

    // 至少有一个 API Key 不为空
    const char* deepseekKey = std::getenv("deepseek_apikey");
    const char* chatgptKey  = std::getenv("chatgpt_apikey");
    const char* geminiKey   = std::getenv("gemini_apikey");

    bool hasApiKey = (deepseekKey != nullptr && std::strlen(deepseekKey) > 0) ||
                     (chatgptKey  != nullptr && std::strlen(chatgptKey)  > 0) ||
                     (geminiKey   != nullptr && std::strlen(geminiKey)   > 0);

    if (!hasApiKey)
    {
        std::cerr << "[ERROR] 至少需要设置一个 API Key 环境变量: "
                  << "deepseek_apikey / chatgpt_apikey / gemini_apikey" << std::endl;
        valid = false;
    }

    // Ollama 配置参数不能为空（如果用户指定了任意一个，则要求全部非空）
    bool hasOllamaModel   = !FLAGS_ollama_model_name.empty();
    bool hasOllamaDesc    = !FLAGS_ollama_model_desc.empty();
    bool hasOllamaEndpoint = !FLAGS_ollama_endpoint.empty();

    if (hasOllamaModel || hasOllamaDesc || hasOllamaEndpoint)
    {
        if (!hasOllamaModel)
        {
            std::cerr << "[ERROR] 指定了 Ollama 配置，但 ollama_model_name 为空" << std::endl;
            valid = false;
        }
        if (!hasOllamaDesc)
        {
            std::cerr << "[ERROR] 指定了 Ollama 配置，但 ollama_model_desc 为空" << std::endl;
            valid = false;
        }
        if (!hasOllamaEndpoint)
        {
            std::cerr << "[ERROR] 指定了 Ollama 配置，但 ollama_endpoint 为空" << std::endl;
            valid = false;
        }
    }

    return valid;
}

// ============================================================
// 从环境变量读取 API Key
// ============================================================ 
static void LoadAPIKeysFromEnv(ai_chat_server::ServerConfig& config)
{
    const char* key = nullptr;

    key = std::getenv("deepseek_apikey");
    if (key != nullptr) config.deepseekAPIKey = key;

    key = std::getenv("chatgpt_apikey");
    if (key != nullptr) config.chatgptAPIKey = key;

    key = std::getenv("gemini_apikey");
    if (key != nullptr) config.geminiAPIKey = key;
}

// ============================================================
// main 函数
// ============================================================
int main(int argc, char** argv)
{
    // 在 gflags 解析之前，先检查 -v / --version 和 -h / --help
    // 因为我们需要自定义这些输出内容
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] != nullptr)
        {
            std::string arg(argv[i]);
            if (arg == "-v" || arg == "--version")
            {
                PrintVersion();
                return 0;
            }
            if (arg == "-h" || arg == "--help")
            {
                PrintHelpMessage();
                return 0;
            }
        }
    }
    std::cout << "进入main" << std::endl;
    // 解析 gflags（包括命令行和 --flagfile 传入的配置文件）
    google::ParseCommandLineFlags(&argc, &argv, true);

    // ---- 参数安全检测 ----
    if (!ValidateConfig())
    {
        std::cerr << "配置校验失败，程序退出。" << std::endl;
        return 1;
    }

    // ---- 初始化日志系统 ----
    std::cout << "初始化log" << std::endl;
    spdlog::level::level_enum logLevel = StringToLogLevel(FLAGS_log_level);
    ns_logger::Logger::initLogger("AIChatServer", "logs/ai_chat_server.log", logLevel);
    std::cout << "初始化日志成功" << std::endl;

    INFO("AIChatServer v{} 启动中 ...", kVersion);

    // ---- 组装 ServerConfig ----
    ai_chat_server::ServerConfig config;
    config.ip          = FLAGS_ip;
    config.port        = FLAGS_port;
    config.temperature = FLAGS_temperature;
    config.maxTokens   = FLAGS_max_tokens;
    INFO("基础ServerConfig初始化成功");
    config.ollamaModelName = FLAGS_ollama_model_name;
    config.ollamaModelDesc = FLAGS_ollama_model_desc;
    config.ollamaEndpoint  = FLAGS_ollama_endpoint;
    INFO("ollamaServerConfig初始化成功");
    // 从环境变量加载 API Key
    LoadAPIKeysFromEnv(config);
    INFO("环境变量中加载api key成功");
    INFO("服务器配置: ip={}, port={}, temperature={}, max_tokens={}",
         config.ip, config.port, config.temperature, config.maxTokens);
    INFO("DeepSeek API Key: {}", config.deepseekAPIKey.empty() ? "未设置" : "已设置");
    INFO("ChatGPT  API Key: {}", config.chatgptAPIKey.empty()  ? "未设置" : "已设置");
    INFO("Gemini   API Key: {}", config.geminiAPIKey.empty()   ? "未设置" : "已设置");
    if (!config.ollamaModelName.empty())
    {
        INFO("Ollama 配置: model={}, desc={}, endpoint={}",
             config.ollamaModelName, config.ollamaModelDesc, config.ollamaEndpoint);
    }

    // ---- 创建并启动 ChatServer ----
    ai_chat_server::ChatServer chatServer(config);

    INFO("正在启动 HTTP 服务 {}:{} ...", config.ip, config.port);
    chatServer.start();
    INFO("start成功");

    if (!chatServer.isRunning())
    {
        CRI("AIChatServer 启动失败！");
        return 1;
    }

    // ---- 等待退出信号（主线程不阻塞服务器运行） ----
    // 服务器在 ChatServer::start() 内部已通过独立线程运行，
    // 主线程使用 sigwait 等待 SIGINT/SIGTERM 实现优雅关闭，避免 busy-wait
    {
        sigset_t signal_set;
        sigemptyset(&signal_set);
        sigaddset(&signal_set, SIGINT);
        sigaddset(&signal_set, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &signal_set, nullptr);

        INFO("AIChatServer 已成功启动，监听地址: {}:{}", config.ip, config.port);
        INFO("访问 http://{}:{} 查看服务", config.ip, config.port);
        INFO("按 Ctrl+C 停止服务");

        int sig;
        sigwait(&signal_set, &sig);
        INFO("收到信号 {}，正在停止服务...", sig);
    }

    chatServer.stop();
    INFO("AIChatServer 已停止。");
    return 0;
}
