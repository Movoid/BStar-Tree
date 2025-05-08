cmake_minimum_required(VERSION 3.25)
project(B-Trees VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE Legacy_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/legacy/*.cpp)
add_executable(Legacy ${Legacy_SOURCES})
target_include_directories(Legacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Legacy)

file(GLOB_RECURSE B_epsilon_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/be/*.cpp)
add_executable(B_epsilon ${B_epsilon_SOURCES})
target_include_directories(B_epsilon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/be)
