cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include_directories(.)

# 编译可执行程序, mulThreadBarrierUseSem
set(SRC_mulThreadBarrierUseSem "${CMAKE_CURRENT_SOURCE_DIR}/mulThreadBarrierUseSem.cpp")
add_executable(${PROJECT_NAME}_mulThreadBarrierUseSem ${SRC_mulThreadBarrierUseSem})

# 编译可执行程序, mulThreadBarrierUseApi_thread_barrier_wait
set(SRC_mulThreadBarrierUseApi_thread_barrier_wait "${CMAKE_CURRENT_SOURCE_DIR}/mulThreadBarrierUseApi_thread_barrier_wait.cpp")
add_executable(${PROJECT_NAME}_mulThreadBarrierUseApi_thread_barrier_wait ${SRC_mulThreadBarrierUseApi_thread_barrier_wait})
