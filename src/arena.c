#include "arena.h"

Arena* init_arena() {
    Arena* arena = calloc(1, sizeof(Arena));
    arena->current_bucket = 0;
    arena->pos = 0;
    arena->block_meta = NULL;
    
    return arena;    
}

void deinit_arena(Arena* arena) {
    for (size_t i = 0; i <= arena->current_bucket; ++i)
	free(arena->buckets[i]);
}

void* amalloc(Arena* arena, size_t nmeb) {
    if (nmeb == 0) {
	return NULL;
    }

    if (arena->buckets[arena->current_bucket] == NULL) {
	arena->buckets[arena->current_bucket] = malloc(sizeof(char) * BUCKET_SIZE);
	assert(arena->buckets[arena->current_bucket] != NULL);	
    }

    unsigned char* memory;
    BMeta* block = find_free_block(arena, nmeb);

    if (block != NULL) {
	block->is_free = 0;
	// NOTE(sam): Since we add the size of a BMeta when returning, we only reinterp the block pointer here.
	memory = (unsigned char*)block;
    } else {
	memory = arena->buckets[arena->current_bucket] + arena->pos;
	arena->pos += sizeof(BMeta) + nmeb;
	while ((arena->pos & 7) != 0)
	    ++arena->pos;

	if (arena->pos >= (BUCKET_SIZE - 1)) {
	    arena->pos = 0;
	    ++arena->current_bucket;
	    if (arena->current_bucket == 64) {
		// TODO(sam): error message here instead of an assert(false)
		assert(false);
	    }	    
	    arena->buckets[arena->current_bucket] = malloc(sizeof(char) * BUCKET_SIZE);
	    assert(arena->buckets[arena->current_bucket] != NULL);
	}

	block = (BMeta*)memory;
	block->next = NULL;
	block->data_size = nmeb;
	block->is_free = 0;
	block->magic = 0x77;
	    
	insert_new_block(arena, block);
    }
	
    return (void*)(memory + sizeof(BMeta));
}

void* acalloc(Arena* arena, size_t size, size_t nmeb) {
    void* memory = amalloc(arena, size * nmeb);
    memset(memory, 0, size * nmeb);
    return memory;
}

void* arealloc(Arena* arena, void* ptr, size_t nmeb) {
    if (ptr == NULL) {
	return ptr;
    }

    BMeta* current = (BMeta*)((unsigned char*)ptr - sizeof(BMeta));
    assert(current->magic == 0x77);

    if (nmeb == 0) {
	afree(arena, ptr);
	return NULL;
    }

    if (current->next == NULL) {
	// NOTE(sam): no copy needed in this case, this was the last allocation and can be safely
	// expanded.
	current->data_size = nmeb;
	return ptr;
    } else {
	// NOTE(sam): in this case, the allocation was somewhere other than the end.
	// we allocate a new block of memory of new desired size, and copy over the
	// old memory to it. the old block is also marked as free for re-use.
	void* memory = amalloc(arena, nmeb);
	memcpy(memory, ptr, nmeb);
	afree(arena, ptr);
	return memory;
    }
}

void afree(Arena* arena, void* ptr) {
    if (ptr == NULL) {
	return;
    }

    // NOTE(sam): A BMeta struct is inserted right before the requested memory, so the ptr minus the size of
    // a BMeta struct will get the start of the BMeta struct.
    BMeta* current = (BMeta*)((unsigned char*)ptr - sizeof(BMeta));
    assert(current->magic == 0x77);
    current->is_free = 1;
}

BMeta* find_free_block(Arena* arena, size_t data_size) {
    BMeta* free_block = arena->block_meta;

    if (free_block == NULL)
	return NULL;
    
    while (true) {
	if (free_block->data_size == data_size && free_block->is_free == 1)
	    return free_block;
	else if (free_block->next != NULL)
	    free_block = free_block->next;
	else
	    break;
    }

    return NULL;
}

void insert_new_block(Arena* arena, BMeta* block) {
    if (arena->block_meta == NULL) {
	arena->block_meta = block;
    } else {
	BMeta* tail = arena->block_meta;
	while (tail->next != NULL) {
	    tail = tail->next;
	}
	tail->next = block;
    }
}
