
#include "stdio.h"
#include <iostream>

#include "math/include/mul.h"
#include "math/include/add.h"
#include "str/include/mystr.h"

 
int main(int argc, char *argv[])
{

    int x = 2, y = 3;
    printf("%d * %d = %d\n", x, y, mul(x, y));
    printf("%d + %d = %d\n", x, y, add(x, y));
    
    std::string s1("hello");
    std::string s2("world");
    std::cout << s1 << " + " << s2 << " = " << strjoin(s1, s2) << std::endl;;
    return 0;
}