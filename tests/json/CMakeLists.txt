cmake_minimum_required(VERSION 3.0)

project(json) 

include_directories(.)
include_directories(../../3rdtools/include)

add_executable(serialization serialization.cpp) 
add_executable(deserialization deserialization.cpp) 