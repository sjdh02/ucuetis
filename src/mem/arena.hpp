#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>

#ifdef _WIN64
#pragma pack(push, 1)
struct BMeta {
    BMeta* next = nullptr;
    size_t data_size;
    int free = 0;
};
#pragma pack(pop)
#elif __unix
struct __attribute__ ((packed)) BMeta {
    BMeta* next = nullptr;
    size_t data_size;
    int free = 0;
};
#endif

class UcMemArena {
public:
    unsigned char* m_memory;
    size_t m_pos;
    size_t m_allocated;
    BMeta* block_meta;
    BMeta* find_free_block(size_t nmeb);
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
	
	auto memory = m_memory + m_pos;
	m_pos += sizeof(BMeta) + (sizeof(T) * nmeb);
	while ((m_pos & 7) != 0)
	    ++m_pos;
	
	if (m_pos >= m_allocated) {
	    m_memory = static_cast<unsigned char*>(realloc(m_memory, sizeof(char) * (m_allocated * 2)));
	    assert(m_memory != nullptr);
	    m_allocated *= 2;
	}

	BMeta* block = reinterpret_cast<BMeta*>(memory);
	block->data_size = sizeof(T);

	if (block_meta == nullptr) {
	    block_meta = block;
	} else {
	    BMeta* tail = block_meta;	    
	    while (tail->next != nullptr) {
		tail = tail->next;
	    }
	    tail->next = block;
	}
	
	return reinterpret_cast<T>(memory + sizeof(BMeta));
    };

    template <typename T>
    void afree(T* ptr) {
	if (ptr == nullptr) {
	    return;
	}
	
	// A BMeta block is inserted right before the memory, so the ptr minus the size of
	// a BMeta struct will get the start of the BMeta struct.
	BMeta* current = reinterpret_cast<BMeta*>(ptr - sizeof(BMeta));
	current->free = 1;
    };
    ~UcMemArena();
};
