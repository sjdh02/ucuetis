#include "arena.hpp"

UcMemArena::UcMemArena() : m_buckets{ nullptr }, m_current_bucket(0), m_pos(0), m_block_meta(nullptr) {}

UcMemArena::~UcMemArena() {
    for (size_t i = 0; i <= m_current_bucket; ++i)
	free(m_buckets[i]);
}

void* UcMemArena::amalloc(size_t nmeb) {
    if (nmeb == 0) {
	return nullptr;
    }

    if (m_buckets[m_current_bucket] == nullptr) {
	m_buckets[m_current_bucket] = static_cast<unsigned char*>(malloc(sizeof(char) * BUCKET_SIZE));
	assert(m_buckets[m_current_bucket] != nullptr);	
    }

    unsigned char* memory;
    BMeta* block = find_free_block(nmeb);

    if (block != nullptr) {
	block->free = 0;
	// NOTE(sam): Since we add the size of a BMeta when returning, we only reinterp the block pointer here.
	memory = reinterpret_cast<unsigned char*>(block);
    } else {
	memory = m_buckets[m_current_bucket] + m_pos;
	m_pos += sizeof(BMeta) + nmeb;
	while ((m_pos & 7) != 0)
	    ++m_pos;

	if (m_pos >= (BUCKET_SIZE - 1)) {
	    m_pos = 0;
	    ++m_current_bucket;
	    if (m_current_bucket == 64) {
		// TODO(sam): error message here instead of an assert(false)
		assert(false);
	    }	    
	    m_buckets[m_current_bucket] = static_cast<unsigned char*>(malloc(sizeof(char) * BUCKET_SIZE));
	    assert(m_buckets[m_current_bucket] != nullptr);
	}

	block = reinterpret_cast<BMeta*>(memory);
	block->next = nullptr;
	block->data_size = nmeb;
	block->free = 0;
	block->magic = 0x77;
	    
	insert_new_block(block);
    }
	
    return static_cast<void*>(memory + sizeof(BMeta));	
}

void* UcMemArena::acalloc(size_t size, size_t nmeb) {
    auto memory = amalloc(size * nmeb);
    memset(memory, 0, size * nmeb);
    return memory;
}

void UcMemArena::afree(void* ptr) {
    if (ptr == nullptr) {
	return;
    }

    // NOTE(sam): A BMeta struct is inserted right before the requested memory, so the ptr minus the size of
    // a BMeta struct will get the start of the BMeta struct.
    BMeta* current = reinterpret_cast<BMeta*>(reinterpret_cast<unsigned char*>(ptr) - sizeof(BMeta));
    assert(current->magic == 0x77);
    current->free = 1;
}

BMeta* UcMemArena::find_free_block(size_t data_size) {
    BMeta* free_block = m_block_meta;

    if (free_block == nullptr)
	return nullptr;
    
    while (true) {
	if (free_block->data_size == data_size && free_block->free == 1)
	    return free_block;
	else if (free_block->next != nullptr)
	    free_block = free_block->next;
	else
	    break;
    }

    return nullptr;
}

void UcMemArena::insert_new_block(BMeta* block) {
    if (m_block_meta == nullptr) {
	m_block_meta = block;
    } else {
	BMeta* tail = m_block_meta;
	while (tail->next != nullptr) {
	    tail = tail->next;
	}
	tail->next = block;
    }
}
