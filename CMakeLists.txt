cmake_minimum_required(VERSION 3.10)
project(SimpleWebServer)

# 设置 C++ 编译标准为 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 将 src/include 目录添加到头文件搜索路径中
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/test)

# 收集 src/impl 文件夹下的所有源文件
file(GLOB IMPL_SOURCES "${PROJECT_SOURCE_DIR}/src/impl/*.cpp")

# 如果 main.cpp 在项目根目录下
file(GLOB MAIN_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 生成可执行文件，将所有源文件加入编译
add_executable(server ${IMPL_SOURCES} ${MAIN_SOURCES})

# 构建测试目标，包含测试代码和实现文件（例如 threadpool.cpp）
add_executable(test_threadpool 
    src/test/test_threadpool.cpp
    src/impl/logger.cpp
    src/impl/threadpool.cpp
    # 如有其他测试相关文件，也可以添加
)

# 查找线程库并链接
find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
target_link_libraries(test_threadpool Threads::Threads)
