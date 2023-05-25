#include "lrucache.hpp"

int main(int argc, char **argv)
{

    LRU<int, int> lru(5);
    int value = -1;

    lru.put(7, 777);
    if (!lru.get(1, value)) {
        std::cout << "get key = 1 failed\n";
    }

    for (int i = 0; i < 9; ++i) {
        lru.put(i, i);
    }

    size_t size = lru.size();
    std::cout << "size = " << size << std::endl;

    lru.show();

    return 0;
}
