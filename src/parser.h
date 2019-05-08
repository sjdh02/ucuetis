#pragma once

#include <assert.h>
#include "alltypes.h"
#include "arena.h"
#include "error.h"
#include "scanner.h"

typedef struct {
    Tokenizer* tokenizer;
    Arena* allocator;
    ErrorStream* estream;
} Parser;

Parser* init_parser(Tokenizer* tokenizer, Arena* allocator, ErrorStream* estream);

UcExpr* get_expr(Parser* parser);
UcExpr* extract_val(Parser* parser);
UcExpr* extract_body(Parser* parser);
UcExpr* extract_list(Parser* parser);
UcExpr* parse_function_call(Parser* parser);
UcExpr* parse_function_decl(Parser* parser);

// NOTE(sam) @HACK: This is a *really* hacky way to take arguments for this, but it does work.
bool check_token(Parser* parser, Token token, enum TypeTag tag, uint64_t enum_or_num);
bool check_tag(Parser* parser, Token token, enum TypeTag tag);
