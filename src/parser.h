#pragma once

#include <assert.h>

#include "../mem/arena.h"
#include "../error/error.h"
#include "../scanner/scanner.h"

typedef enum {
    NumLit, StrLit,
    Ident, Builtin,
} ActiveValue;

struct Value {
    ActiveValue active;    
    union {
	uint64_t NumLit;
	char* StrLit;
	char* Ident;
	Lexeme Builtin;
    } data;
};

typedef struct {
    struct UcArgList* next;
    char* ident;
    Lexeme type;
} UcArgList;

typedef enum {
    Value, Assign,
    List, Boolean,
    Math, Pipe,
    If, While,
    For, Yield,
    FunctionDecl, FunctionCall,
    BadExpr,    
} ActiveExpr;

typedef struct {
    ActiveExpr active;    
    union {
	Value Value;

	/*
	 * Assign expressions consist of an assignment identifier, and a value to place in it. This value can be
	 * any expression, which allows for both the function definition syntax of ucuetis as well as the ability
	 * to assign (almost) anything to an identifier. 
	 */
	struct {
	    struct UcExpr* ident;
	    struct UcExpr* value;
	} Assign;

	/* 
	 * Lists are easily represented as a singly-linked list, with each node holding the value of the list at
	 * position n and a pointer to the value of the list at n + 1. 
	 */
	struct {
	    struct UcExpr* next;
	    struct UcExpr* value;
	} List;

	/*
	 * Boolean expressions simply contain their operators and operands. As with most other types, they take two
	 * UcExpr as operands.
	 */
	struct {
	    Lexeme op;
	    struct UcExpr* lhs;
	    struct UcExpr* rhs;
	} Boolean;

	/*
	 * Math expressions are defined in the same manner as boolean expressions.
	 */
	struct {
	    Lexeme op;
	    struct UcExpr* lhs;
	    struct UcExpr* rhs;
	} Math;

	/*
	 * A pipe expression simply consists of a destination and source.
	 */
	struct {
	    struct UcExpr* dest;
	    struct UcExpr* source;
	} Pipe;

	/*
	 * An if expression is a conditional (this *should always* be a Boolean expression) and some statements that
	 * make up the body. These are represented with the List expression type, as it lends itself quite well to
	 * the concept.
	 */
	struct {
	    struct UcExpr* cond;
	    struct UcExpr* stmts;
	} If;

	/*
	 * The only difference between a while expression and if expression: behavior.
	 */
	struct {
	    struct UcExpr* cond;
	    struct UcExpr* stmts;
	} While;

	/*
	 * A for expression consists of an iteration target, and a list of statements that make up the body.
	 */
	struct {
	    struct UcExpr* target;
	    struct UcExpr* stmts;
	} For;

	/*
	 * A yield expression is extrememly basic, as it only consists of a expression to return from the current scope.
	 */
	struct UcExpr* Yield;

	/*
	 * A function defintion contains a return type, arguments list, and list of statements in the body.
	 */
	struct {
	    UcArgList* args;
	    struct UcExpr* stmts;
	    Lexeme r_type;
	} FunctionDecl;

	/*
	 * A function call needs a target function identifier and list of arguments.
	 */
	struct {
	    char* ident;
	    struct UcExpr* args;
	} FunctionCall;
    } data;
} UcExpr;

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
bool check_token(ActiveToken tag, uint64_t enum_or_num, char* ident_or_str);
