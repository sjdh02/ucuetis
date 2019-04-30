#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>

struct BMeta {
    BMeta* next;
    size_t data_size;
    int free;
    int magic;
};

class UcMemArena {
    unsigned char* m_memory;
    size_t m_pos;
    size_t m_allocated;
    BMeta* m_block_meta;
    BMeta* find_free_block(size_t data_size);
    void insert_new_block(BMeta* block);
public:
    UcMemArena();
    


    void* amalloc(size_t nmeb);
    void* acalloc(size_t size, size_t nmeb);
    void afree(void* ptr);
    ~UcMemArena();
};
