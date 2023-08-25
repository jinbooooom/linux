g++ -shared -fPIC MathPlugin.cpp -o plugin.so
g++ main.cpp -ldl -o main

# 这两行代码是用来编译和链接 C++ 插件以及主程序的命令。

#     g++ -shared -fPIC MathPlugin.cpp -o plugin.so
#         g++: 调用 GNU 编译器 g++
#         -shared: 生成共享库（动态链接库）的选项
#         -fPIC: 生成位置无关代码的选项，用于确保共享库的可移植性
#         MathPlugin.cpp: 插件源文件的文件名
#         -o plugin.so: 指定输出的共享库文件的名称为 "plugin.so"

#     这条命令会将 MathPlugin.cpp 编译成一个共享库文件 plugin.so，其中包含了插件的实现。

#     g++ main.cpp -ldl -o main
#         g++: 调用 GNU 编译器 g++
#         main.cpp: 主程序的源文件的文件名
#         -ldl: 链接动态加载库（libdl），用于使用动态库加载相关函数
#         -o main: 指定输出的可执行文件的名称为 "main"

#     这条命令会将 main.cpp 编译成一个可执行文件 main，并将动态库加载函数所需的库链接到可执行文件中。
