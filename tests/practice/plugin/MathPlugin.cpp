// 这是 MathPlugin.cpp 源文件
#include "IPlugin.h"

class MathPlugin : public IPlugin {
public:
    int add(int a, int b) override {
        return a + b;
    }
    
    int subtract(int a, int b) override {
        return a - b;
    }
};

// 创建插件实例
extern "C" IPlugin* createPlugin() {
    return new MathPlugin();
}

// 销毁插件实例
extern "C" void destroyPlugin(IPlugin* plugin) {
    delete plugin;
}
