cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include_directories(.)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ SRC)

add_executable(${PROJECT_NAME} ${SRC}) 
# add_dependencies(${PROJECT_NAME} ${MY_LIB_NAME})
# target_link_libraries(${PROJECT_NAME} PRIVATE ${MY_LIB_NAME})
