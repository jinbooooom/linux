#include "enum.h"
#include <iostream>

BETTER_ENUM(NUM, int, Zero = 0, One, Two, Three, Four, Five, Ten = 10, Thousand = 1000)

int main()
{
    NUM zero = NUM::Zero;
    NUM one = NUM::One;
    printf("zero: %s, value = %d, index = %lu\n", zero._to_string(), zero._to_integral(), zero._to_index());
    printf("one: %s, value = %d, index = %lu\n", one._to_string(), one._to_integral(), one._to_index());
    printf("two: %s, value = %d, index = %lu\n", (+NUM::Two)._to_string(), (+NUM::Two)._to_integral(),
           (+NUM::Two)._to_index());

    // 从字符串构建枚举变量 Ten
    NUM ten = NUM::_from_string("Ten");
    printf("ten: %s, value = %d, index = %lu\n", ten._to_string(), ten._to_integral(), ten._to_index());
    // 从数字构建枚举变量 Thousand
    auto thousand = NUM::_from_integral(1000);
    printf("thousand: %s, value = %d, index = %lu\n", thousand._to_string(), thousand._to_integral(),
           thousand._to_index());

    // 迭代器遍历
    for (size_t index = 0; index < NUM::_size(); ++index) {
        NUM channel = NUM::_values()[index];
        const char *name = NUM::_names()[index];
        std::cout << name << ": " << channel._to_integral() << std::endl;
    }

    // 支持 switch 语句
    NUM num = NUM::Five;
    int n = -1;
    switch (num) {
    case NUM::One:
        n = 1;
        break;
    case NUM::Five:
        n = 5;
        break;
    case NUM::Ten:
        n = 10;
        break;
    default:
        n = -1;
        break;
    }
    printf("switch case %s, n = %d\n", num._to_string(), n);

    return 0;
}
