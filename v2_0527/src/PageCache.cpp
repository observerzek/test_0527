#include "../include/PageCache.h"


namespace MemoryPool
{

void* PageCache::allocate(size_t page_nums){
    auto it = page_pool_.lower_bound(page_nums);
    
    if(it != page_pool_.end()){
        std::shared_ptr<PageNode> page_node = it->second;
        if(it->first == page_nums){
            page_pool_[it->first] = page_node->next;
            
        }
        else if(it->first > page_nums){
            std::shared_ptr<PageNode> new_node = std::make_shared<PageNode>();
            void* split_add = reinterpret_cast<char*>(page_node->page_add) +  page_nums * PAGE_SIZE;
            new_node->page_add = split_add;
            new_node->page_nums = it->first - page_nums;
            if(page_pool_.find(new_node->page_nums) == page_pool_.end()){
                page_pool_[new_node->page_nums] = nullptr;
            }
            new_node->next = page_pool_[new_node->page_nums];
            page_pool_[new_node->page_nums] = new_node;


            page_node->page_nums = page_nums;

        }
        page_node->next = nullptr;
        return page_node->page_add;
    }



}



    
} //namespace MemoryPool