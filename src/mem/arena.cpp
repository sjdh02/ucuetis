#include "arena.hpp"

UcMemArena::UcMemArena() : m_memory(nullptr), m_pos(0), m_allocated(0), m_block_meta(nullptr) {}

UcMemArena::~UcMemArena() {
    free(m_memory);
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
