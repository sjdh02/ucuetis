#pragma once

#include <assert.h>
#include <stdio.h>
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
    size_t scope_level;
} Analyzer;

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream);
void analyze(Analyzer* analyzer);
void analyze_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_assign_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_math_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_if_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_while_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_for_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_yield_expr(Analyzer* analyzer, UcExpr* expr);
void analyze_function_decl(Analyzer* analyzer, UcExpr* expr);
void analyze_function_call(Analyzer* analyzer, UcExpr* expr);

int find_symbol(Analyzer* analyzer, char* needle, size_t scope_level);
void push_symbol(Analyzer* analyzer, Symbol symbol);
void drop_symbol(Analyzer* analyzer);
void build_symbol_table(Analyzer* analyzer);
