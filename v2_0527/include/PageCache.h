#pragma once
#include <map>
#include "utility.h"
#include <mutex>
#include <memory>

namespace MemoryPool
{

constexpr size_t PAGE_SIZE = 4096;

class PageCache{
private:
    struct PageNode{
        std::shared_ptr<PageNode> next;
        size_t page_nums;
        void* page_add;
    };

    using NodePtr = std::shared_ptr<PageNode>;

    std::map<size_t, NodePtr> page_pool_;
    std::map<void*, NodePtr> address_to_node_;
    std::mutex mutex_;

public:
    // 函数PageCache前忘记加static
    // 导致 PageCache::static调用失败
    static PageCache& getInstance(){
        static PageCache page_cache;
        return page_cache;
    }

    void* allocate(size_t page_nums);  
    void  deallocate(void* prt, size_t bytes);

private:
    PageCache() = default;

    void* getMemoryFromSys(size_t bytes);

};




} // namespace MemoryPool