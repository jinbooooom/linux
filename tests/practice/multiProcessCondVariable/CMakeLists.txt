cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include_directories(.)

# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

# 排除含 main 函数的文件
list(FILTER SRC EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/testCond.cpp")
list(FILTER SRC EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/testLock.cpp")
# message(WARNING "SRC = ${SRC}")

# 编译可执行程序，添加 testCond.cpp 文件到 SRC 列表
set(SRC_TESTCOND ${SRC})
list(APPEND SRC_TESTCOND "${CMAKE_CURRENT_SOURCE_DIR}/testCond.cpp")
add_executable(${PROJECT_NAME}_testCond ${SRC_TESTCOND}) 

# 编译可执行程序，添加 testLock.cpp 文件到 SRC 列表
set(SRC_TESTLOCK ${SRC})
list(APPEND SRC_TESTLOCK "${CMAKE_CURRENT_SOURCE_DIR}/testLock.cpp")
add_executable(${PROJECT_NAME}_testLock ${SRC_TESTLOCK}) 



