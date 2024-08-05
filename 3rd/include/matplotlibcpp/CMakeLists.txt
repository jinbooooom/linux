# 声明要求的 cmake 最低版本
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 11)

project(TEST)

# 查找python库版本
# find_package(PythonLibs 3.6)


# 指定头文件路径
set(PYTHON3.6_INLCUDE_DIRS "/usr/include/python3.6")

# 添加头文件到工程
include_directories(
    ${PYTHON3.6_INLCUDE_DIRS}
    )
#这样配置可以使得matplotlib.h中找到python.h文件，通常这样设置就可以。


# 添加一个可执行程序
add_executable(test test.cpp)

# 添加相关库文件链接到工程
target_include_directories(test PRIVATE ${PYTHON2_INCLUDE_DIRS})
# target_link_libraries(test ${PYTHON_LIBRARIES})
target_link_libraries(test /usr/lib/x86_64-linux-gnu/libpython3.6m.so)

