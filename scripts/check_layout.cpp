#include <iostream>
#include <cstddef>
#include "axeon/bytecode.hpp"

using namespace kio;

int main() {
    std::cout << "Size of Value: " << sizeof(Value) << std::endl;
    std::cout << "Offset of as: " << offsetof(Value, as) << std::endl;
    std::cout << "Offset of number: " << offsetof(Value, as.number) + offsetof(Value, as) - offsetof(Value, as) << std::endl; // simplified
    Value v;
    std::cout << "Offset of as.number direct: " << (char*)&v.as.number - (char*)&v << std::endl;
    return 0;
}
