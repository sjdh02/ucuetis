#pragma once

#include "../mem/arena.hpp"

#define ABORT_ERROR_THRESH 64

class ErrorStream {
    UcMemArena* m_allocator;
    char* m_errors[ABORT_ERROR_THRESH];
public:
    ErrorStream();
    void pop_error();
    void push_error();
    ~ErrorStream() = default;
};
