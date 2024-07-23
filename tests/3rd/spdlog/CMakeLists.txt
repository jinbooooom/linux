cmake_minimum_required(VERSION 3.0)

project(spdlog) 

include_directories(.)
include_directories(../../3rdtools/include)

add_executable(testSpdlog testSpdlog.cpp)  