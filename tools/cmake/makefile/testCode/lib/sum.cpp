#include <vector>

int sum(const std::vector<int> &vec)
{
    int res = 0;
    for (int i = 0; i < vec.size(); ++i)
            res += vec[i];
    return res; 
}