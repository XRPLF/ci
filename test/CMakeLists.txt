cmake_minimum_required(VERSION 3.15)
project(example CXX)

find_package(ZLIB REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)
