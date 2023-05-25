/*
 * File:   lrucache.hpp
 * Author: Jinbo
 *
 * Created on 2023/05/25
 */

#ifndef __LRUCACHE_HPP__
#define __LRUCACHE_HPP__

#include <cstddef>
#include <iostream>
#include <list>
#include <stdexcept>
#include <unordered_map>

template <typename K, typename V>
class LRU final
{
public:
    typedef typename std::pair<K, V> kv_pair_t;
    typedef typename std::list<kv_pair_t>::iterator it_t;

    explicit LRU(size_t maxSize) : mMaxSize(maxSize) {}
    LRU() = delete;
    ~LRU() = default;

    void put(const K &key, const V &value)
    {
        auto it = mCacheMap.find(key);
        mkvList.push_front(kv_pair_t(key, value));
        if (it != mCacheMap.end()) {
            mkvList.erase(it->second);
            mCacheMap.erase(it);
        }
        mCacheMap[key] = mkvList.begin();

        if (mCacheMap.size() > mMaxSize) {
            auto last = mkvList.end();
            last--;
            mCacheMap.erase(last->first);
            mkvList.pop_back();
        }
    }

    bool get(const K &key, V &value)
    {
        auto it = mCacheMap.find(key);
        if (it == mCacheMap.end()) {
            std::cout << "There is no such key in cache\n";
            return false;
        } else {
            mkvList.splice(mkvList.begin(), mkvList, it->second);
            value = it->second->second;
            return true;
        }
    }

    bool exists(const K &key) const
    {
        return mCacheMap.find(key) != mCacheMap.end();
    }

    size_t size() const
    {
        return mCacheMap.size();
    }

    void show() const  // for debug
    {
        for (auto it = mkvList.begin(); it != mkvList.end(); ++it) {
            std::cout << "(K, V) = (" << it->first << ", " << it->second << ")\n";
        }
    }

private:
    std::list<kv_pair_t> mkvList;
    std::unordered_map<K, it_t> mCacheMap;
    size_t mMaxSize;
};

#endif /* __LRUCACHE_HPP__ */
