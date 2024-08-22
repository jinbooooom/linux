## 调试和性能分析工具

- gdb：是Linux下最常用的调试器，可以在命令行下调试程序，支持反汇编，查看变量和内存地址。
- valgrind：是一套用于调试内存问题的工具集，包括内存泄漏检测、越界访问检测等功能。可以通过其中的memcheck工具检查内存错误。
- strace：可以跟踪程序的系统调用和信号。
- ltrace：可以跟踪程序的库调用。
- perf：是一个Linux下性能分析工具，可以用来分析CPU、内存、I/O等方面的性能问题。
- gdbserver：是一个远程调试工具，可以让开发人员在远程机器上进行调试。

## 3rd 中的 C/C++ 开源库

|                             库名                             | 描述                                                         |
| :----------------------------------------------------------: | ------------------------------------------------------------ |
| **[better-enums](https://github.com/aantron/better-enums)**  | 枚举变量与字符串转换的 header-only 库，要求C++11（含）以上   |
|   **[magic_enum](https://github.com/Neargye/magic_enum)**    | 枚举变量与字符串转换的 header-only 库，要求C++17（含）以上   |
| **[matplotlib-cpp](https://github.com/lava/matplotlib-cpp)** | 基于流行的matplotlib构建的极其简单但功能强大的纯头文件C++绘图库 |
|           **[MD5](https://github.com/pod32g/MD5)**           | MD5算法的C实现                                               |
|        **[spdlog](https://github.com/gabime/spdlog)**        | 非常快，仅头文件/编译的C++日志库                             |
|         **[json](https://github.com/nlohmann/json)**         | [nlohmann](https://github.com/nlohmann)的 C++11 JSON 库，提供直观的语法 |
|   **[flatbuffers](https://github.com/google/flatbuffers)**   | FlatBuffers是一个跨平台的序列化库，旨在最大限度地提高内存效率。它允许您直接访问序列化数据，而无需首先解析/解包它，同时仍然具有很好的向前/向后兼容性。 |

