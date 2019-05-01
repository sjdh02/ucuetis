#pragma once

#include <cstdio>
#include "../mem/arena.hpp"

#define MAX_ERR 64
#define MAX_ERR_LEN 128

enum class ErrorKind {
    UnknownCharacter,
    UnexpectedToken,
    UnexpectedEOS,    
};

class UcErrorStream {
    UcMemArena* m_allocator;
    char m_errors[MAX_ERR][MAX_ERR_LEN];
    size_t m_current_error;
public:
    UcErrorStream(UcMemArena* p_allocator);
    void push_error(ErrorKind kind, char* module, size_t pos);
    char* pop_error();
    void report_errors();
    ~UcErrorStream() = default;
};
