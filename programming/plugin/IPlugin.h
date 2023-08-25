
#ifndef __IPLUGIN_H__
#define __IPLUGIN_H__

class IPlugin
{
public:
    virtual ~IPlugin() {}
    virtual int add(int a, int b) = 0;
    virtual int subtract(int a, int b) = 0;
};

// 此处带virtual关键字的函数，都是接口（interface) ，
// 方便在装载不同动态库的时候，可实现同一接口但根据动态库不同来返回不同的信息内容（多态化）。

#endif //  __IPLUGIN_H__
