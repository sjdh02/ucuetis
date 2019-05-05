#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arena.h"
//#include "../error/error.h"

enum Lexeme {
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

enum ActiveToken {
    Lexeme, NumLit,
    StrLit, Ident,
};

typedef struct {
    enum ActiveToken active;
    union {
	enum Lexeme lexeme;
	uint64_t num_lit;
	char* str_lit;
	char* ident;
    } data;
} Token;

typedef struct {
    const char* data;
    Arena* allocator;
    //ErrorSTream* estream; TODO(sam)
    uint32_t len;
    uint32_t pos;
    uint32_t line;
    uint32_t column;
    uint32_t last_len;
} Tokenizer;

Tokenizer* init_tokenizer();

Token get_token(Tokenizer* tokenizer);
inline Token get_current_token(Tokenizer* tokenizer);
Token peek_token(Tokenizer* tokenizer);
inline void step_back(Tokenizer* tokenizer);
inline void skip_token(Tokenizer* tokenizer);
inline size_t get_pos(Tokenizer* tokenizer);
inline bool is_at_end(Tokenizer* tokenizer);

// This might be markable as static?
Token parse_ident(Tokenizer* tokenizer);
Token parse_num(Tokenizer* tokenizer);
Token parse_str(Tokenizer* tokenizer);
Token parse_multi(Tokenizer* tokenizer);
Token parse_single(Tokenizer* tokenizer);
inline void skip_whitespace(Tokenizer* tokenizer);

inline bool is_delim(char c);
inline bool is_digit(char c);
inline bool is_alpha(char c);
