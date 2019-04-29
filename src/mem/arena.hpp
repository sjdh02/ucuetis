#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>

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
    
    template <typename T>
    T amalloc(size_t nmeb) {
	if (sizeof(T) < 1) {
	    return nullptr;
	}
	
	if (m_memory == nullptr) {
	    m_memory = static_cast<unsigned char*>(malloc(sizeof(char) * 16392));
	    assert(m_memory != nullptr);
	    m_allocated = 16392;
	}

	unsigned char* memory;
	BMeta* block = find_free_block(sizeof(T));

	if (block != nullptr) {
	    block->free = 0;
	    // NOTE(sam): Since we add the size of a BMeta when returning, we only reinterp the block pointer here.
	    memory = reinterpret_cast<unsigned char*>(block);
	} else {
	    memory = m_memory + m_pos;
	    m_pos += sizeof(BMeta) + (sizeof(T) * nmeb);
	    while ((m_pos & 7) != 0)
		++m_pos;
	    
	    if (m_pos >= m_allocated) {
		m_memory = static_cast<unsigned char*>(realloc(m_memory, sizeof(char) * (m_allocated * 2)));
		assert(m_memory != nullptr);
		m_allocated *= 2;
	    }
	    
	    block = reinterpret_cast<BMeta*>(memory);
	    block->next = nullptr;
	    block->data_size = sizeof(T);
	    block->free = 0;
	    block->magic = 0x77;
	    
	    insert_new_block(block);
	}
	
	return reinterpret_cast<T>(memory + sizeof(BMeta));	
    };

    void afree(void* ptr) {
	if (ptr == nullptr) {
	    return;
	}

	// NOTE(sam): A BMeta struct is inserted right before the requested memory, so the ptr minus the size of
	// a BMeta struct will get the start of the BMeta struct.
	BMeta* current = reinterpret_cast<BMeta*>(static_cast<unsigned char*>(ptr) - sizeof(BMeta));
	assert(current->magic == 0x77);
	current->free = 1;
    };
    ~UcMemArena();
};
