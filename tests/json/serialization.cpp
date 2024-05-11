#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    // create an empty structure (null)
    json j;

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;

    // add a Boolean that is stored as bool
    j["happy"] = true;

    // add a string that is stored as std::string
    j["name"] = "Niels";

    // add another null object by passing nullptr
    j["nothing"] = nullptr;

    // add an object inside the object
    j["answer"]["everything"] = 42;

    // add an array that is stored as std::vector (using an initializer list)
    j["list"] = {100, -9, 88};

    // add another object (using an initializer list of pairs)
    j["object"] = {{"currency", "USD"}, {"value", 42.99}};

    // 增加自定义结构体数组
    json objectArray;
    int arraySize = 3;
    for (int i = 0; i < arraySize; ++i)
    {
        json object;
        object["currency"] = std::string("currency") + std::to_string(i);
        object["value"]    = i;

        json objectArrayElement;
        objectArrayElement["name"]   = std::string("name") + std::to_string(i);
        objectArrayElement["object"] = object;
        objectArray.emplace_back(std::move(objectArrayElement));
    }
    j["objectArray"] = objectArray;

    // write prettified JSON to another file
    std::ofstream o("pretty.json");
    o << std::setw(4) << j << std::endl;

    // instead, you could also write (which looks very similar to the JSON above)
    json j2 = {{"pi", 3.141},
               {"happy", true},
               {"name", "Niels"},
               {"nothing", nullptr},
               {"answer", {{"everything", 42}}},
               {"list", {1, 0, 2}},
               {"object", {{"currency", "USD"}, {"value", 42.99}}}};

    // write prettified JSON to another file
    std::ofstream o2("pretty2.json");
    o2 << std::setw(4) << j2 << std::endl;

    return 0;
}
