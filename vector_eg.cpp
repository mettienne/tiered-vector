#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<int> ve;
    ve.push_back(1);
    ve.push_back(2);
    ve.push_back(3);
    int & s = ve[2];
    printf("s = %d\n", s);
    s = 6;
    printf("%d %d %d\n", ve[0], ve[1], ve[2]);
    return 0;
}