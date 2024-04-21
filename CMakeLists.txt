cmake_minimum_required(VERSION 3.25)
project(Webserver)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include/mysql)  # 添加 MySQL 头文件的路径

# 所有源文件
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 添加可执行文件
add_executable(Webserver ${SRC_LIST}
        lock/lock.h
        log/block_queue.h
        log/log.h
        log/log.cpp
        CGlmysql/sql_connection_pool.h
        CGlmysql/sql_connection_pool.cpp
        timer/lst_timer.h
        timer/lst_timer.cpp
        threadpool/threadpool.h
        http/http_conn.h
        http/http_conn.cpp)

# 添加 MySQL 客户端库的链接指令
link_directories(/usr/lib/x86_64-linux-gnu)  # 添加 MySQL 库文件路径
target_link_libraries(Webserver mysqlclient ssl crypto resolv m)  # 链接 MySQL 客户端库及其他依赖库
