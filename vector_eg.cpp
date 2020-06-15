#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<int> ve;
    ve.reserve(5);
    printf("size of ve = %lu, capacity = %lu\n", ve.size(), ve.capacity());
    ve.insert(ve.begin(), 100);
    for (int j = 0; j < 5; ++j) {
        printf("%d ", ve[j]);
    }
    ve[0] = 0;
    ve[1] = 1;
    for (int j = 0; j < 5; ++j) {
        printf("%d ", ve[j]);
    }
    printf("\n");
    ve.reserve(10);
    for (int j = 0; j < 5; ++j) {
        printf("%d ", ve[j]);
    }
    printf("\n");

    ve.push_back(1);
    ve.push_back(2);
    ve.push_back(3);
    int & s = ve[2];
    printf("s = %d\n", s);
    s = 6;
    printf("%d %d %d\n", ve[0], ve[1], ve[2]);
    ve.clear();
    ve.resize(16);
    ve.resize(0);
    printf("size of ve = %lu, capacity = %lu\n", ve.size(), ve.capacity());
    for (int i = 0; i < 8; ++i) {
        ve.push_back(i);
        printf("size of ve = %lu, capacity = %lu\n", ve.size(), ve.capacity());
    }
    ve.resize(20);
    ve.resize(9);
    for (int j = 0; j < 8; ++j) {
        printf("%d ", ve[j]);
    }
    return 0;
}