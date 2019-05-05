#pragma once

#include <assert.h>

#include "arena.h"
#include "alltypes.h"
//#include "../error/error.h"
#include "scanner.h"


typedef struct {
    Tokenizer* tokenizer;
    Arena* allocator;
//    ErrorStream* estream; TODO(sam)    
} Parser;

Parser* init_parser(Tokenizer* tokenizer, Arena* allocator);

UcExpr* extract_val(Parser* parser);
UcExpr* extract_body(Parser* parser);
UcExpr* extract_list(Parser* parser);
UcExpr* parse_function_call(Parser* parser);
UcExpr* parse_function_decl(Parser* parser);

// NOTE(sam) @HACK: This is a *really* hacky way to take arguments for this, but it does work.
bool check_token(Parser* parser, enum TokenTag tag, uint64_t enum_or_num, char* ident_or_str);
