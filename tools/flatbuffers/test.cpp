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

struct Picture_t {
    std::string path;
    uint64_t size;
    std::vector<uint8_t> data;
};

struct TestObj_t {
    uint64_t id;
    Picture_t picture;
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
    auto list = builder->CreateVector(vec);  // vector

    // Create picture
    std::string picPath("./qianxun.jpg");
    auto picturePath = builder->CreateString(picPath);
    auto pictureSize = getFileSize(picPath);
    std::vector<uint8_t> vecPicData(pictureSize, 0);
    printf("before %d\n", vecPicData[0]);
    int fd = -1;
    fd = open(picPath.c_str(), O_RDWR, 0777);
    if (fd < 0) {
        std::cout << "open picture" << picPath << " failed!" << std::endl;
        return nullptr;
    }
    if (pictureSize != read(fd, vecPicData.data(), pictureSize)) {
        std::cout << "read data from picture" << picPath << "failed!" << std::endl;
        return nullptr;
    }
    close(fd);
    auto pictureData = builder->CreateVector(vecPicData);
    auto picture = TestFlat::CreatePicture(*builder, picturePath, pictureSize, pictureData);
    
    // Create flat buffer inner type
    auto id = 1;
    auto flag = 1;
    auto kv = KV(100, 99.99);  // struct
    // table
    auto mloc = CreateTestObj(*builder, id, picture, flag, list, &kv);
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
        std::cout << "create file " << file << " failed!" << std::endl;
        return -1;
    }

    if (size != write(fd, ptr, size)) {
        std::cout << "write data to file " << file << " failed!" << std::endl;
        return -1;
    }

    close(fd);
}

int deserialise(const TestFlat::TestObj *obj, TestObj_t &testobj)
{
    // printf(" -----------------------  deserialise start -------------------------\n");

    // std::cout << obj->id() << std::endl;
    // std::cout << obj->picture()->path()->c_str() << std::endl;
    // std::cout << obj->picture()->size() << std::endl;
    // std::cout << obj->flag() << std::endl;
    // for (size_t i = 0; i < obj->list()->size(); i++) {
    //     std::cout << obj->list()->Get(i) << std::endl;
    // }

    // std::cout << obj->kv()->key() << std::endl;
    // std::cout << obj->kv()->value() << std::endl;

    // printf(" -----------------------  deserialise end ----------------------------\n");

    testobj.id = obj->id();
    testobj.picture.path = obj->picture()->path()->c_str();
    testobj.picture.size = obj->picture()->size();
    // for (size_t i = 0; i < obj->picture()->size(); ++i) {
    //     testobj.picture.data.push_back(obj->picture()->data()->Get(i));
    // } 
    testobj.picture.data.resize(obj->picture()->size(), 0);
    memcpy(testobj.picture.data.data(), obj->picture()->data()->data(), obj->picture()->size() * sizeof(uint8_t));
    testobj.flag = obj->flag();

    // 1.当 list 是指针的话，赋值很简单，但 obj 释放了，这就是野指针
    // testobj.list = const_cast<uint64_t *>(obj->list()->data()); 
    // 2.也可以逐元素赋值
    // for (size_t i = 0; i < obj->list()->size(); ++i) {
    //     testobj.list.push_back(obj->list()->Get(i));
    // }
    // 3. 最方便，直接 memcpy，记得提前 resize vector 的空间
    testobj.list.resize(obj->list()->size(), 0);
    memcpy(testobj.list.data(), obj->list()->data(), obj->list()->size() * sizeof(uint64_t));

    testobj.kv.key = obj->kv()->key();
    testobj.kv.value = obj->kv()->value();

    return 0;
}

int deserialiseFromBuilder(std::shared_ptr<flatbuffers::FlatBufferBuilder> builder /*const void *src*/,
                           TestObj_t &testobj)
{
    std::cout << "call deserialise from builder" << std::endl;

    char *ptr = (char *) builder->GetBufferPointer();
    uint64_t size = builder->GetSize();
    auto obj = GetTestObj((uint8_t *) ptr);

    return deserialise(obj, testobj);
}

int deserialiseFromFile(const std::string &file, TestObj_t &testobj)
{
    std::cout << "call deserialise from file" << std::endl;

    auto size = getFileSize(file);
    if (0 == size) {
        std::cout << "file " << file << " not exist!" << std::endl;
        return -1;
    } else {
        std::cout << file << " size = " << size << std::endl;
    }

    char *ptr = new char[size];

    int fd = -1;
    fd = open(file.c_str(), O_RDWR, 0777);
    if (fd < 0) {
        std::cout << "open file " << file << " failed!" << std::endl;
        return -1;
    }

    if (size != read(fd, ptr, size)) {
        std::cout << "read data from file " << file << "failed!" << std::endl;
        return -1;
    }

    auto obj = GetTestObj(ptr);
    //auto obj = TestFlat::GetSizePrefixedTestObj(ptr);
    deserialise(obj, testobj);

    delete[] ptr;
    close(fd);

    return 0;
}

int main()
{
    TestObj_t obj;

#if 0
    auto builder = serialize();
    deserialiseFromBuilder(builder, obj);
#else
    serializeToFile();
    deserialiseFromFile("data.dat", obj);
#endif

    std::cout << "id = " << obj.id << std::endl;
    std::string deserialisePic = std::string("./deserialise.jpg");
    std::cout << "save picture to " << deserialisePic << ", original picture is " << obj.picture.path << ", size = " << obj.picture.size << std::endl;
    std::cout << "flag = " << (int)obj.flag << std::endl; // 强制转换，避免 std::cout 把 flag 当做字符打印
    for (size_t i = 0; i < obj.list.size(); i++) {
        std::cout << "list[" << i << "] = " << obj.list[i] << std::endl;
    }
    std::cout << "(k, v) = (" << obj.kv.key << ", " << obj.kv.value << ")" << std::endl;

    // save picture
    int fd = -1;
    fd = open(deserialisePic.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0777);
    if (fd < 0) {
        std::cout << "save picture " << deserialisePic << " failed!" << std::endl;
        return -1;
    }
    if (obj.picture.size != write(fd, obj.picture.data.data(), obj.picture.size)) {
        std::cout << "write data to picture " << deserialisePic << " failed!" << std::endl;
        return -1;
    }
    close(fd);

    return 0;
}