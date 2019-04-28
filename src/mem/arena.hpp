#pragma once

#include <cstdlib>
#include <cassert>

class UcMemArena {
public:
    unsigned char* m_memory;
    size_t m_pos;
    size_t m_allocated;
public:
    UcMemArena();
    
    template <typename T>
    T* amalloc(size_t nmeb) {
	if (m_memory == nullptr) {
	    m_memory = static_cast<unsigned char*>(malloc(sizeof(char) * 8196));
	    assert(m_memory != nullptr);
	    m_allocated = 8196;
	}

	auto memory = m_memory + m_pos;
	m_pos += sizeof(T) * nmeb;
	while ((m_pos & 7) != 0)
	    ++m_pos;

	if (m_pos >= m_allocated) {
	    m_memory = static_cast<unsigned char*>(realloc(m_memory, sizeof(char) * (m_allocated * 2)));
	    assert(m_memory != nullptr);
	    m_allocated *= 2;
	}
	
	return reinterpret_cast<T*>(memory);
    };

    ~UcMemArena();
};
