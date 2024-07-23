
cmake_minimum_required (VERSION 3.12)
project (mytest)

add_subdirectory(3rd/md5)
add_subdirectory(practice/getopt_long)

message(WARNING "MY_LIB_NAME ${MY_LIB_NAME}")

