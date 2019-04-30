#include "arena.hpp"

UcMemArena::UcMemArena() : m_memory(nullptr), m_pos(0), m_allocated(0), m_block_meta(nullptr) {}



UcMemArena::~UcMemArena() {
    free(m_memory);
}

void* UcMemArena::amalloc(size_t nmeb) {
    if (nmeb == 0) {
	return nullptr;
    }
	
    if (m_memory == nullptr) {
	m_memory = static_cast<unsigned char*>(malloc(sizeof(char) * 16392));
	assert(m_memory != nullptr);
	m_allocated = 16392;
    }

    unsigned char* memory;
    BMeta* block = find_free_block(nmeb);

    if (block != nullptr) {
	block->free = 0;
	// NOTE(sam): Since we add the size of a BMeta when returning, we only reinterp the block pointer here.
	memory = reinterpret_cast<unsigned char*>(block);
    } else {
	memory = m_memory + m_pos;
	m_pos += sizeof(BMeta) + nmeb;
	while ((m_pos & 7) != 0)
	    ++m_pos;
	    
	if (m_pos >= m_allocated) {
	    m_memory = static_cast<unsigned char*>(realloc(m_memory, sizeof(char) * (m_allocated * 2)));
	    assert(m_memory != nullptr);
	    m_allocated *= 2;
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
