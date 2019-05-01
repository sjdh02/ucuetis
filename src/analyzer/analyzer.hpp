#pragma once

#include "../parser/parser.hpp"
#include "../mem/arena.hpp"
#include "../error/error.hpp"

class Analyzer {
    Parser* m_parser;
    UcMemArena* m_allocator;
    UcErrorStream* m_stream;
public:
    Analyzer(Parser* p_parser, UcMemArena* p_allocator, UcErrorStream* p_stream) :
	m_allocator(p_allocator), m_stream(p_stream), m_parser(p_parser) {};
    
    ~Analyzer() = default;
};
