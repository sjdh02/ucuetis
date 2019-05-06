#pragma once

#include "alltypes.h"
#include "parser.h"
#include "arena.h"
#include "error.h"

typedef struct {
    Parser* parser;
    Arena* allocator;
    ErrorStream* estream;
    Symbol** symbol_table;
    size_t pos;
} Analyzer;

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream);


