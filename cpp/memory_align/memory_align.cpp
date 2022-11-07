
#include <iostream>

struct x1 {
    int  i1;
    char c1;
    char c2;
};

struct x2 {
    char c1;
    char c2;
    int  i1;
};

struct x3 {
    char c1;
    int  i1;
    char c2;
};

struct x5 {
    uint64_t i64_1;
};

struct x4 {
    char     c1;
    uint16_t i16_1;
};

struct x6 {
    __int128_t i64_1;
};

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

struct MyPackedData {
    char Data1;
    long Data2;
    char Data3;
};

#pragma pack(pop) /* restore original alignment from stack */

int main() {
    std::cout << "x1 " << sizeof(x1) << std::endl;
    std::cout << "x2 " << sizeof(x2) << std::endl;
    std::cout << "x3 " << sizeof(x3) << std::endl;
    std::cout << "x4 " << sizeof(x4) << std::endl;
    std::cout << "x5 " << sizeof(x5) << std::endl;
    std::cout << "x6 " << sizeof(x6) << std::endl;
    std::cout << "MyPackedData " << sizeof(MyPackedData) << std::endl;
}
