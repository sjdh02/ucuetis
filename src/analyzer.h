#pragma once

#include <assert.h>
#include "alltypes.h"
#include "parser.h"
#include "arena.h"
#include "error.h"

typedef struct {
    Parser* parser;
    Arena* allocator;
    ErrorStream* estream;
    Symbol* symbol_table;
    size_t st_pos;
    size_t st_len;
} Analyzer;

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream);
void analyze(Analyzer* analyzer);
void build_symbol_table(Analyzer* analyzer);
