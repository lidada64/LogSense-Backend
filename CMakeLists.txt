cmake_minimum_required(VERSION 3.15)
project(LogSense_Backend)  # project name


#---解决 VS Code 找不到头文件的问题 ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

# --- 自动下载依赖库 (CPR 和 JSON) ---
include(FetchContent)

# 1. 下载 CPR (网络库)
FetchContent_Declare( # reserve the library
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.10.4
)
FetchContent_MakeAvailable(cpr) # order

# 2. 下载 JSON 库
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# --- 编译设置 ---
add_executable(LogSense_Backend main.cpp)

# --- 链接库文件 ---
target_link_libraries(LogSense_Backend PRIVATE cpr::cpr nlohmann_json::nlohmann_json)