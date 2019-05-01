#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "../mem/arena.hpp"
#include "../error/error.hpp"

enum class Lexeme {
    // scoping characters
    LParen, RParen,
    LBracket, RBracket,
    LBrace, RBrace,
    // comparisons
    Lt, Gt,
    Eq, Bang,
    // math operators
    Plus, Minus,
    Div, Mul,
    // delimiters
    Semicolon, Colon,
    Bar, Comma,
    Dot,
    // require peeking
    Assign, Pipe,
    LtOrEq, GtOrEq,
    Neq, RType,
    // reserved keywords
    Out, In,
    ErrOut, If,
    Else, While,
    For, Fn,
    True, False,
    It, Yield,
    Loop, Break,
    // types
    Num, Str,
    List,

    EOS,
};

struct Token {
    enum class Active { Lexeme, NumLit, StrLit, Ident };
    Active active;
    union {
	Lexeme Lexeme;
	uint64_t NumLit;
	char* StrLit;
	char* Ident;
    } data;
};

class Tokenizer {
    const char* m_data;
    UcMemArena* m_allocator;
    UcErrorStream* m_stream;
    uint32_t m_len;
    uint32_t m_pos;
    int m_line;
    int m_column;
    int m_last_len;

    Token parse_ident();
    Token parse_num();
    Token parse_str();
    Token parse_multi();
    Token parse_single();
    void skip_whitespace();
public:
    Tokenizer(const char* p_data, UcMemArena* p_allocator, UcErrorStream* p_stream) :
	m_data(p_data), m_allocator(p_allocator), m_stream(p_stream),
	m_len(strlen(p_data)), m_pos(0), m_line(1), m_column(0), m_last_len(0) {};
    
    Token get_next();
    Token get_current();
    Token peek_token();
    void step_back();
    void skip_token();
    size_t get_pos();
    bool is_at_end();
    ~Tokenizer() = default;
};

bool is_delim(char c);
bool is_digit(char c);
bool is_alpha(char c);
