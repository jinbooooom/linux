#include "IPlugin.h"
#include <dlfcn.h>  // 动态库加载相关头文件
#include <iostream>

int main()
{
    // 加载插件动态库，可以将虚函数不同的实现版本做成不同的动态库，调用同一接口根据加载不同的动态库来实现不同的功能
    // RTLD_LAZY标志告诉系统在运行时仅解析和连接共享库中实际使用到的符号。这意味着在加载共享库时，并不会立即解析和检查所有的符号依赖关系，而是在第一次使用到某个符号时才会进行解析。
    // RTLD_LAZY标志的主要优点是延迟符号解析和链接，可以减少启动时间和内存开销。尤其在大型项目或有多个共享库的情况下，如果不是每个符号都被使用到，采用RTLD_LAZY标志可以提高性能。
    // 然而，需要注意的是，由于延迟解析符号的特性，如果某些符号未正确定义或存在依赖性问题，可能会导致运行时错误。因此，在使用RTLD_LAZY标志时，确保共享库及其依赖的符号都是正确定义和链接的至关重要。
    void *handle = dlopen("./plugin.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "无法加载插件: " << dlerror() << std::endl;
        return -1;
    }

    // 获取插件的创建和销毁函数指针
    typedef IPlugin *(*CreatePlugin)();
    typedef void (*DestroyPlugin)(IPlugin *);

    CreatePlugin createPlugin = reinterpret_cast<CreatePlugin>(dlsym(handle, "createPlugin"));
    DestroyPlugin destroyPlugin = reinterpret_cast<DestroyPlugin>(dlsym(handle, "destroyPlugin"));

    if (!createPlugin || !destroyPlugin) {
        std::cerr << "无法找到函数: " << dlerror() << std::endl;
        dlclose(handle);  // 关闭动态库
        return -1;
    }

    // 创建插件实例
    IPlugin *plugin = createPlugin();

    // 使用插件执行操作
    int result_add = plugin->add(5, 3);
    int result_subtract = plugin->subtract(10, 4);

    std::cout << "加法结果: " << result_add << std::endl;
    std::cout << "减法结果: " << result_subtract << std::endl;

    // 销毁插件实例
    destroyPlugin(plugin);

    // 关闭动态库
    dlclose(handle);

    return 0;
}
