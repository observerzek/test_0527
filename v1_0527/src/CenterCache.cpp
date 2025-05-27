#include "../include/CenterCache.h"
#include "../include/PageCache.h"
#include <atomic>
#include <thread>
#include <cstring>
#include <algorithm>
#include <math.h>

namespace MemoryPool
{

 

void* CenterCache::allocate(size_t bytes, size_t nums){
    if(bytes == 0) bytes = 8;

    size_t index = SizeClass::getIndex(bytes);

    while(center_list_lock_[index].test_and_set(std::memory_order_acquire)){
        std::this_thread::yield();
    }

    void* memory = center_list_[index].load(std::memory_order_relaxed);
    if(center_list_nums_[index] < nums){
        memory = getMemoryFromPage(bytes);
    }

    // 找到分割块
    void* split_block = memory;
    size_t count = 1;
    while(count < nums){
        split_block = getNextBlock(split_block);
        count++;
    }
    // 找到保留的内存块
    void* old_memory = getNextBlock(split_block);
    // 断开连接
    breakBlockConnect(split_block);


    center_list_[index].store(old_memory, std::memory_order_relaxed);
    center_list_nums_[index] -= nums;
    center_list_lock_[index].clear(std::memory_order_release);

    return memory;

}

void* CenterCache::getNextBlock(void* current){
    void* next = *reinterpret_cast<void**>(current);
    return next;
}

void CenterCache::breakBlockConnect(void* current){
    *reinterpret_cast<void**>(current) = nullptr;

}


void* CenterCache::getMemoryFromPage(size_t bytes){
    size_t index = SizeClass::getIndex(bytes);
    bytes = SizeClass::byteAlignment(bytes);

    size_t page_nums = (bytes + PAGE_SIZE - 1)  / PAGE_SIZE;
    page_nums = std::max(page_nums, size_t(MIN_PAGE));
    void* memory = PageCache::getInstance().allocate(page_nums);
    void* current = memory;
    if(current){
        size_t steps = (page_nums * PAGE_SIZE) / bytes;
        size_t gap = bytes / ALIGNMENT;
        for(int step = 1; step < steps; step++){
            void* next = reinterpret_cast<char*>(current) + bytes;
            *reinterpret_cast<void**>(current) = next;
            current = next;
        }
        current = center_list_[index].load(std::memory_order_relaxed);
        center_list_nums_[index] += steps;
        center_list_[index].store(current, std::memory_order_relaxed);
    }
    return center_list_[index].load(std::memory_order_relaxed);
}

void CenterCache::deallocate(void* ptr, size_t bytes){
    if(!ptr) return;
    size_t index = SizeClass::getIndex(bytes);
    
    while(center_list_lock_[index].test_and_set(std::memory_order_acquire)){
        std::this_thread::yield();
    }

    void* last_block = ptr;
    void* next = getNextBlock(last_block);
    size_t count = 0;
    while(next){
        count++;
        last_block = next;
        next = getNextBlock(last_block);
    }
    
    center_list_nums_[index] += count;
    *reinterpret_cast<void**>(last_block) = center_list_[index].load(std::memory_order_relaxed);
    center_list_[index].store(ptr, std::memory_order_relaxed);

    center_list_lock_[index].clear(std::memory_order_release);

}

} //namespace MemoryPool