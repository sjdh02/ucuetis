#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>

#define BUCKET_SIZE 64000

struct BMeta {
    BMeta* next;
    size_t data_size;
    int free;
    int magic;
};

class UcMemArena {
    // NOTE(sam): Currently, there is a hard limit of 64 buckets. This gives a total
    // available memory amount of 4Mb.
    unsigned char* m_buckets[64];
    size_t m_current_bucket;
    size_t m_pos;
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
