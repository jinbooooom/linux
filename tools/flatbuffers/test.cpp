#include "test_generated.h"

#include <iostream>
#include <memory>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct KV_t {
    uint64_t key;
    double value;
};

struct TestObj_t {
    uint64_t id;
    std::string picture_path;
    uint32_t picture_size;  // 72336
    uint8_t flag;
    std::vector<uint64_t> list;
    // uint64_t *list;
    KV_t kv;
};

using namespace TestFlat;

uint64_t getFileSize(const std::string &path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0) {
        return 0;
    }
    return (uint64_t) buf.st_size;
}

std::shared_ptr<flatbuffers::FlatBufferBuilder> serialize()
{
    auto builder = std::make_shared<flatbuffers::FlatBufferBuilder>();

    /////////// Serialize //////////
    // Create list
    std::vector<uint64_t> vec;
    for (size_t i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    // Create flat buffer inner type
    auto id = 1;
    std::string picture("./qianxun.jpg");
    auto picture_path = builder->CreateString(picture);
    auto picture_size = getFileSize(picture);
    ;                                        // the size of picture
    auto list = builder->CreateVector(vec);  // vector
    auto flag = 1;
    auto kv = KV(100, 99.99);  // struct
    // table
    auto mloc = CreateTestObj(*builder, id, picture_path, picture_size, flag, list, &kv);
    builder->Finish(mloc);

    return builder;
}

int serializeToFile()
{
    std::string file("data.dat");

    auto builder = serialize();
    char *ptr = (char *) builder->GetBufferPointer();
    uint64_t size = builder->GetSize();
    std::cout << file << " size = " << size << std::endl;

    int fd = -1;
    fd = open(file.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0777);
    if (fd < 0) {
        std::cout << "create file" << file << " failed!" << std::endl;
        return -1;
    }

    if (size != write(fd, ptr, size)) {
        std::cout << "write data to file" << file << " failed!" << std::endl;
        return -1;
    }

    close(fd);
}

int deSerialise(const TestFlat::TestObj *obj, TestObj_t &testobj)
{
    printf(" -----------------------  DeSerialise start -------------------------\n");

    std::cout << obj->id() << std::endl;
    std::cout << obj->picture_path()->c_str() << std::endl;
    std::cout << obj->picture_size() << std::endl;
    std::cout << obj->flag() << std::endl;
    for (size_t i = 0; i < obj->list()->size(); i++) {
        std::cout << obj->list()->Get(i) << std::endl;
    }

    std::cout << obj->kv()->key() << std::endl;
    std::cout << obj->kv()->value() << std::endl;

    printf(" -----------------------  DeSerialise end ----------------------------\n");

    testobj.id = obj->id();
    testobj.picture_path = obj->picture_path()->c_str();
    testobj.picture_size = obj->picture_size();
    testobj.flag = obj->flag();

    printf("start\n");
    // testobj.list = const_cast<uint64_t *>(obj->list()->data()); //当 list 是指针的话，赋值很简单，但 obj
    // 释放了，这就是野指针
    for (size_t i = 0; i < obj->list()->size(); i++) {
        testobj.list.push_back(obj->list()->Get(i));
    }

    printf("end\n");

    testobj.kv.key = obj->kv()->key();
    testobj.kv.value = obj->kv()->value();

    return 0;
}

int deSerialiseFromBuilder(std::shared_ptr<flatbuffers::FlatBufferBuilder> builder /*const void *src*/, TestObj_t &testobj)
{
    std::cout << "call deSerialiseFromBuilder" << std::endl;

    char *ptr = (char *) builder->GetBufferPointer();
    uint64_t size = builder->GetSize();

    ////////// Deserialize //////////
    auto obj = GetTestObj((uint8_t *) ptr);

    return deSerialise(obj, testobj);
}

int deSerialiseFromFile(const std::string &file, TestObj_t &testobj)
{
    std::cout << "call deSerialiseFromFile" << std::endl;

    auto size = getFileSize(file);
    if (0 == size)
    {
        std::cout << "file" << file << " not exist!" << std::endl;
        return -1;
    }

    char *ptr = new char[size];

    int fd = -1;
    fd = open(file.c_str(), O_RDWR, 0777);
    if (fd < 0) {
        std::cout << "open file" << file << " failed!" << std::endl;
        return -1;
    }

    if (size != read(fd, ptr, size)) {
        std::cout << "read data from file" << file << "failed!" << std::endl;
        return -1;
    }

    auto obj = TestFlat::GetSizePrefixedTestObj(ptr);
    deSerialise(obj, testobj);

    delete[] ptr;
    close(fd);

    return 0;
}

int main()
{
    TestObj_t obj;

#if 0
    auto builder = serialize();
    deSerialiseFromBuilder(builder, obj);
#else
    serializeToFile();
    deSerialiseFromFile("data.dat", obj);
#endif

    printf(" -----------------------  main ----------------------------\n");
    std::cout << obj.id << std::endl;
    std::cout << obj.picture_path << std::endl;
    std::cout << obj.picture_size << std::endl;
    std::cout << obj.flag << std::endl;
    for (size_t i = 0; i < obj.list.size(); i++) {
        std::cout << "obj.list[" << i << "] = " << obj.list[i] << std::endl;
    }

    std::cout << obj.kv.key << std::endl;
    std::cout << obj.kv.value << std::endl;

    printf(" -----------------------  main ----------------------------\n");

    return 0;
}