#pragma once

#include "../mem/arena.hpp"
#include "../scanner/scanner.hpp"

struct Value {
    enum class Active { Num, Str, Ident, Builtin };
    union {
	uint64_t num;
	char* str;
	char* ident;
	Lexeme builtin;
    } data;
};

struct UcExpr {
    enum class Active {
	Value, List,
	Boolean, Assign,
	Math, Pipe,
	If, While,
	For, Yield,
	Function, FunctionCall
    };
    union {
	Value value;
	// anonymous structs for the other types.
    } expr;
};

class Parser {
    Tokenizer* m_tokenizer;
    UcMemArena* m_allocator;
public:
    Parser(Tokenizer* p_tokenizer, UcMemArena* p_allocator) : m_tokenizer(p_tokenizer), m_allocator(p_allocator) {};
    ~Parser() = default;
}
