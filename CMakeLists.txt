cmake_minimum_required(VERSION 3.25)
project(Webserver)

include_directories(${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(Webserver ${SRC_LIST}
        lock/lock.h
        log/block_queue.h
        log/log.h
        log/log.cpp)
