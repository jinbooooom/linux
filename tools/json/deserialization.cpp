#include <common.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    std::ifstream jfile("pretty.json");
    if (!jfile.is_open())
    {
        std::cout << "open json file failed" << std::endl;
        return -1;
    }

    // serialization and assignment
    json jdata = json::parse(jfile);
    data_t data;
    data.pi       = jdata["pi"].get<double>();
    data.happy    = jdata["happy"].get<bool>();
    data.name     = jdata["name"].get<std::string>();
    auto jnothing = jdata["nothing"];
    if (jnothing.size() != 0)
    {
        data.nothing.value = jnothing["value"].get<int>();
    }
    else
    {
        std::cout << "jdata[\"nothing\"] is nullptr" << std::endl;
    }
    data.answer.everything = jdata["answer"]["everything"].get<int>();
    data.list              = jdata["list"].get<std::vector<int>>();
    data.object.currency   = jdata["object"]["currency"].get<std::string>();
    data.object.value      = jdata["object"]["value"].get<double>();

    auto jobjectArray = jdata["objectArray"];
    for (auto &e : jobjectArray.items())
    {
        object_array_t objectElement;
        auto jvalue                   = e.value();
        objectElement.name            = jvalue["name"].get<std::string>();
        objectElement.object.currency = jvalue["object"]["currency"].get<std::string>();
        objectElement.object.value    = jvalue["object"]["value"].get<double>();
        data.objectArray.push_back(objectElement);
    }

    std::cout << "pi = " << data.pi << std::endl
              << "happy = " << data.happy << std::endl
              << "name = " << data.name << std::endl
              << "answer.everything = " << data.answer.everything << std::endl
              << "object.currency = " << data.object.currency << std::endl
              << "object.value = " << data.object.value << std::endl;

    for (int i = 0; i < data.list.size(); ++i)
    {
        std::cout << "list[" << i << "] = " << data.list[i] << std::endl;
    }

    for (int i = 0; i < data.objectArray.size(); ++i)
    {
        printf("objectArray[%d]: name = %s, object.currency = %s, object.value = %lf\n", i, data.objectArray[i].name.c_str(),
               data.objectArray[i].object.currency.c_str(), data.objectArray[i].object.value);
    }

    return 0;
}