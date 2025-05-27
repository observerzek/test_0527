#pragma once
// #include <cstddef>
#include <iostream>


namespace MemoryPool
{

constexpr size_t ALIGNMENT = 8;
// 最大为256KB
constexpr size_t MAX_BYTES = 256 * 1024;
// 表长为 32K
constexpr size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT;


constexpr size_t BLOCK_NUMS_MAX = 64;

constexpr size_t MIN_PAGE = 8;   

constexpr size_t PAGE_SIZE = 4096;

class SizeClass{
public:

    static size_t byteAlignment(size_t bytes){
        return (getIndex(bytes) + 1) * ALIGNMENT;
    }

    // 0~8 byte -> 0
    // 9~16 byte -> 1
    // 17~24 byte -> 2
    static size_t getIndex(size_t bytes){
        return (bytes + ALIGNMENT - 1) / ALIGNMENT - 1;
    }



};



} // namespace MemoryPool