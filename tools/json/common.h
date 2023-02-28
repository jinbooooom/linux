#ifndef __JSON_COMMON_H__
#define __JSON_COMMON_H__

#include <string>
#include <vector>

struct nothing_t
{
    int value;
};

struct answer_t
{
    int everything;
};

struct object_t
{
    std::string currency;
    double value;
};

struct object_array_t
{
    std::string name;
    object_t object;
};

struct data_t
{
    double pi;
    bool happy;
    std::string name;
    nothing_t nothing;
    answer_t answer;
    std::vector<int> list;
    object_t object;
    std::vector<object_array_t> objectArray;
};

#endif