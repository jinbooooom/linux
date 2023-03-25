#include <iostream>
#include <vector>

#include "all.h"

using namespace std;

int main(int argc, char **argv)
{
    int arr[5] = {1,2,3,4,5};
    vector<int> vec(arr, arr + 5);
    show(vec);
    int b = 3;
    int c = sum(vec);
    int d = oddSum(vec);
    cout << "sum:" << c << endl;
    cout << "oddSum:" << d << endl;
}