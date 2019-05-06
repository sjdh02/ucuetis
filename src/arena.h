#pragma once

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "alltypes.h"

#define BUCKET_SIZE 64000

typedef struct {
    // NOTE(sam): Currently, there is a hard limit of 64 buckets. This gives a total
    // available memory amount of 4Mb.
    unsigned char* buckets[64];
    size_t current_bucket;
    size_t pos;
    BMeta* block_meta;
} Arena;

Arena* init_arena();
void deinit_arena(Arena* arena);

void* amalloc(Arena* arena, size_t nmeb);
void* acalloc(Arena* arena, size_t size, size_t nmeb);
void afree(Arena* arena, void* ptr);

BMeta* find_free_block(Arena* arena, size_t data_size);
void insert_new_block(Arena* arena, BMeta* block);
