cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include_directories(.)

# 编译可执行程序, crashTriggerSignalToClearsShm
set(SRC_crashTriggerSignalToClearsShm "${CMAKE_CURRENT_SOURCE_DIR}/crashTriggerSignalToClearsShm.cpp")
add_executable(${PROJECT_NAME}_crashTriggerSignalToClearsShm ${SRC_crashTriggerSignalToClearsShm})

# 编译可执行程序, deleteShmMultipleTimes
set(SRC_deleteShmMultipleTimes "${CMAKE_CURRENT_SOURCE_DIR}/deleteShmMultipleTimes.cpp")
add_executable(${PROJECT_NAME}_deleteShmMultipleTimes ${SRC_deleteShmMultipleTimes})

# 编译可执行程序, createShm1
set(SRC_createShm1 "${CMAKE_CURRENT_SOURCE_DIR}/createShm1.cpp")
add_executable(${PROJECT_NAME}_createShm1 ${SRC_createShm1})

# 编译可执行程序, createShm2
set(SRC_createShm2 "${CMAKE_CURRENT_SOURCE_DIR}/createShm2.cpp")
add_executable(${PROJECT_NAME}_createShm2 ${SRC_createShm2})


