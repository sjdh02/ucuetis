#pragma once

#include <cassert>

#include "../mem/arena.hpp"
#include "../scanner/scanner.hpp"

struct Value {
    enum class Active { NumLit, StrLit, Ident, Builtin };
    
    Active active;
    
    union {
	uint64_t NumLit;
	char* StrLit;
	char* Ident;
	Lexeme Builtin;
    } data;
};

struct UcArgList {
    UcArgList* next;
    Value data;
};

struct UcExpr {
    enum class Active {
	Value, Assign,
	List, Boolean,
	Math, Pipe,
	If, While,
	For, Yield,
	Function, FunctionCall
    };
    
    Active active;
    
    union {
	Value Value;

	/*
	 * Assign expressions consist of an assignment identifier, and a value to place in it. This value can be
	 * any expression, which allows for both the function definition syntax of ucuetis as well as the ability
	 * to assign (almost) anything to an identifier. 
	 */
	struct {
	    UcExpr* ident;
	    UcExpr* value;
	} Assign;

	/* 
	 * Lists are easily represented as a singly-linked list, with each node holding the value of the list at
	 * position n and a pointer to the value of the list at n + 1. 
	 */
	struct {
	    UcExpr* next;
	    UcExpr* value;
	} List;

	/*
	 * Boolean expressions simply contain their operators and operands. As with most other types, they take two
	 * UcExpr as operands.
	 */
	struct {
	    Lexeme op;
	    UcExpr* lhs;
	    UcExpr* rhs;
	} Boolean;

	/*
	 * Math expressions are defined in the same manner as boolean expressions.
	 */
	struct {
	    Lexeme op;
	    UcExpr* lhs;
	    UcExpr* rhs;
	} Math;

	/*
	 * A pipe expression simply consists of a destination and source.
	 */
	struct {
	    UcExpr* dest;
	    UcExpr* source;
	} Pipe;

	/*
	 * An if expression is a conditional (this *should always* be a Boolean expression) and some statements that
	 * make up the body. These are represented with the List expression type, as it lends itself quite well to
	 * the concept.
	 */
	struct {
	    UcExpr* cond;
	    UcExpr* statements;
	} If;

	/*
	 * The only difference between a while expression and if expression: behavior.
	 */
	struct {
	    UcExpr* cond;
	    UcExpr* statements;
	} While;

	/*
	 * A for expression consists of an iteration target, and a list of statements that make up the body.
	 */
	struct {
	    UcExpr* target;
	    UcExpr* statements;
	} For;

	/*
	 * A yield expression is extrememly basic, as it only consists of a expression to return from the current scope.
	 */
	UcExpr* Yield;

	/*
	 * A function defintion contains a return type, arguments list, and list of statements in the body.
	 */
	struct {
	    Lexeme r_type;
	    UcArgList* arguments;
	    UcExpr* statements;
	} Function;

	/*
	 * A function call needs a target function identifier and list of arguments.
	 */
	struct {
	    char* ident;
	    UcExpr* args;	    
	} FunctionCall;	
    } data;
};

class Parser {
    Tokenizer* m_tokenizer;
    UcMemArena* m_allocator;
    UcExpr* extract_val();
    UcExpr* extract_body();
    UcExpr* extract_list();
    UcExpr* parse_function_call();
    UcExpr* parse_function_decl();
public:
    Parser(Tokenizer* p_tokenizer, UcMemArena* p_allocator) : m_tokenizer(p_tokenizer), m_allocator(p_allocator) {};
    UcExpr* get_expr();
    ~Parser() = default;
};
