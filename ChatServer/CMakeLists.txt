# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(AIChatServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型
set(CMAKE_BUILD_TYPE Debug)

# 添加可执行文件
add_executable(AIChatServer
    main.cc
    ChatSever.cc
)

# 设置可执行文件的输出目录
set_target_properties(AIChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加头文件搜索路径（当前目录和 sdk 头文件目录）
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(/usr/local/include)

# 查找 OpenSSL 库
find_package(OpenSSL REQUIRED)

# 添加 OpenSSL 头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sdk/include)

# 添加 CPPHTTPLIB_OPENSSL_SUPPORT 宏
target_compile_definitions(AIChatServer PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# 链接库
# ai_chat_sdk 是 SDK 的静态库（位于 /usr/local/lib/libai_chat_sdk.a）
# 其他依赖包括 gflags（命令行参数解析）、spdlog（日志）、jsoncpp（JSON 处理）、
# fmt（格式化）、sqlite3（数据库）、OpenSSL（HTTPS 支持）、pthread（线程）
target_link_libraries(AIChatServer
    ai_chat_sdk
    gflags
    spdlog
    jsoncpp
    fmt
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
