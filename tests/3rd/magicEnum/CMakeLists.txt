cmake_minimum_required (VERSION 3.12)

# 获取最后一个目录名），并将其赋值给 PROJECT_NAME 变量
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
# 如果支持 C++17 就编译，不支持就跳过本目标的编译
if(COMPILER_SUPPORTS_CXX17)
    # magic enum 必须要使用 C++17 编译，设置 C++17 标准，只对当前的 CMakeLists.txt 文件及其子目录中的目标有效
    set(CMAKE_CXX_STANDARD 17) 
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include_directories(../../../3rd/include/magicEnum/)

    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ SRC)

    add_executable(${PROJECT_NAME} ${SRC}) 
    add_dependencies(${PROJECT_NAME} ${MY_LIB_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MY_LIB_NAME})
else()
    message(WARNING "Compiler does not support C++17. Skipping ${PROJECT_NAME} compilation.")
endif()
