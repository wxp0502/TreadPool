cmake_minimum_required(VERSION 3.21)
project(TreadPool)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 11)

add_executable(TreadPool src/example.cpp)
