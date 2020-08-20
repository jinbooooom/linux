#include <iostream>
#include <vector>

using namespace std;

void show(const vector<int> &vec)
{
    cout << "vector: " << endl;
    for (int i = 0; i < vec.size(); ++i)
        cout << vec[i] << " ";
    cout << endl;
}