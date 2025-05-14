cmake_minimum_required(VERSION 3.25)
project(B-Trees VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE new_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/new/*.cpp)
add_executable(new ${new_SOURCES})
target_include_directories(new PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/new)
