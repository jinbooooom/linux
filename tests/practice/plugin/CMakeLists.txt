cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include_directories(.)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ LIBRARY_SRC)
# 排除含 main 函数的文件
list(FILTER LIBRARY_SRC EXCLUDE REGEX "main.cpp")

add_library(pluginTest SHARED ${LIBRARY_SRC})
target_link_libraries(pluginTest -ldl)

set(EXE_SRC main.cpp)
add_executable(${PROJECT_NAME} ${EXE_SRC}) 

