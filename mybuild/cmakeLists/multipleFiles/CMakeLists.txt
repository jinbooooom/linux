
message("开始编译")


cmake_minimum_required (VERSION 3.4.1)
project(demo VERSION 2.0)
message("项目跟路径"${PROJECT_SOURCE_DIR})
message("编译路径"${PROJECT_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. DIR_SRCS)
file(GLOB_RECURSE HPP_LIST "*.h") #递归搜索
message(${HPP_LIST})

add_subdirectory(math)
add_subdirectory(str)
include_directories(HPP_LIST)
# 指定生成目标
add_executable(demo ${DIR_SRCS})

# 添加链接库
target_link_libraries(demo mymath mystr)


