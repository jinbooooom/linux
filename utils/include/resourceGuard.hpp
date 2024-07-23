#ifndef __JINBO_RESOURCE_GUARD_HPP__
#define __JINBO_RESOURCE_GUARD_HPP__

#include <functional>

template <typename F>
class ResourceGuard
{
public:
    explicit ResourceGuard(F f) : mF(f){};
    ResourceGuard() = delete;
    ResourceGuard(const ResourceGuard &) = delete;
    ResourceGuard &operator=(const ResourceGuard &) = delete;
    ~ResourceGuard()
    {
        mF;
    }

private:
    F mF;
};

#endif
