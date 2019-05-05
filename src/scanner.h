#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arena.h"
#include "alltypes.h"
//#include "../error/error.h"

typedef struct {
    char* data;
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
Token get_current_token(Tokenizer* tokenizer);
Token peek_token(Tokenizer* tokenizer);
void step_back(Tokenizer* tokenizer);
void skip_token(Tokenizer* tokenizer);
size_t get_pos(Tokenizer* tokenizer);
bool is_at_end(Tokenizer* tokenizer);

// This might be markable as static?
Token parse_ident(Tokenizer* tokenizer);
Token parse_num(Tokenizer* tokenizer);
Token parse_str(Tokenizer* tokenizer);
Token parse_multi(Tokenizer* tokenizer);
Token parse_single(Tokenizer* tokenizer);
void skip_whitespace(Tokenizer* tokenizer);

bool is_delim(char c);
bool is_digit(char c);
bool is_alpha(char c);
