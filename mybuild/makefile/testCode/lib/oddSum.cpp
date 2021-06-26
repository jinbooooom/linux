#include <vector>
#include "../utils/utils.h"

int oddSum(const std::vector<int> &vec)
{
    int res = 0;
    for (int i = 0; i < vec.size(); ++i)
    {
        if (isOdd(vec[i]))
            res += vec[i];
    }
    return res; 
}