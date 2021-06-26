cmake_minimum_required(VERSION 3.4.1)
project(demo)

message("print message:build with debug mode") # 打印消息

#add_library(hello src/main.c)

add_executable(demo src/main.c)