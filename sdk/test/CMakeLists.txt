#设置cmake版本
cmake_minimum_required(VERSION 3.10)
#设置项目名称
project(testProvider)
#设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置构建类型debug
set(CMAKE_BUILD_TYPE Debug)

#添加可执行文件
add_executable(testProvider testProvider.cc
../src/DeepSeekProvider.cc
../src/ChatGPTProvider.cc
../src/GeminiProvider.cc
../src/OllamaLLMProvider.cc
../src/until/mylog.cc
../src/DataManager.cc
../src/SessionManager.cc
../src/ChatSDK.cc
../src/LLMManager.cc)

#添加可执行程序的输出目录
set_target_properties(testProvider PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#添加头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

#查找OpenSSL库
find_package(OpenSSL REQUIRED)

#添加头文件搜索路径
include_directories(${OPENSSL_INCLUDE_DIR}/../include)

#添加CPPHTTPLIB_OPENSSL_SUPPORT宏
target_compile_definitions(testProvider PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

#链接库
target_link_libraries(testProvider jsoncpp fmt spdlog gtest OpenSSL::SSL OpenSSL::Crypto sqlite3)
