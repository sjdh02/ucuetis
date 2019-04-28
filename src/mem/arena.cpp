#include "arena.hpp"

UcMemArena::UcMemArena() : m_memory(nullptr), m_pos(0), m_allocated(0) {}

UcMemArena::~UcMemArena() {
    free(m_memory);
}
