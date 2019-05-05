#pragma once

typedef struct {
    struct BMeta* next;
    size_t data_size;
    int is_free;
    int magic;
} BMeta;

enum TokenTag {
    Lexeme, NumLit,
    StrLit, Ident,
};

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

typedef struct {
    enum TokenTag active;
    union {
	enum Lexeme lexeme;
	uint64_t num_lit;
	char* str_lit;
	char* ident;
    } data;
} Token;

typedef struct {
    enum TokenTag active;    
    union {
	enum Lexeme builtin;
	uint64_t num_lit;
	char* str_lit;
	char* ident;
    } data;
} Value;

typedef struct {
    struct UcArgList* next;
    char* ident;
    enum Lexeme type;
} UcArgList;

enum ExprTag {
    ValueExpr, AssignExpr,
    ListExpr, BoolExpr,
    MathExpr, PipeExpr,
    IfExpr, WhileExpr,
    ForExpr, YieldExpr,
    FunctionDeclExpr, FunctionCallExpr,
    BadExpr,
};

typedef struct {
    enum ExprTag active;    
    union {
	Value value;

	/*
	 * Assign expressions consist of an assignment identifier, and a value to place in it. This value can be
	 * any expression, which allows for both the function definition syntax of ucuetis as well as the ability
	 * to assign (almost) anything to an identifier. 
	 */
	struct {
	    struct UcExpr* ident;
	    struct UcExpr* value;
	} assign_expr;

	/* 
	 * Lists are easily represented as a singly-linked list, with each node holding the value of the list at
	 * position n and a pointer to the value of the list at n + 1. 
	 */
	struct {
	    struct UcExpr* next;
	    struct UcExpr* value;
	} list_expr;

	/*
	 * Boolean expressions simply contain their operators and operands. As with most other types, they take two
	 * UcExpr as operands.
	 */
	struct {
	    enum Lexeme op;
	    struct UcExpr* lhs;
	    struct UcExpr* rhs;
	} boolean_expr;

	/*
	 * Math expressions are defined in the same manner as boolean expressions.
	 */
	struct {
	    enum Lexeme op;
	    struct UcExpr* lhs;
	    struct UcExpr* rhs;
	} math_expr;

	/*
	 * A pipe expression simply consists of a destination and source.
	 */
	struct {
	    struct UcExpr* dest;
	    struct UcExpr* source;
	} pipe_expr;

	/*
	 * An if expression is a conditional (this *should always* be a Boolean expression) and some statements that
	 * make up the body. These are represented with the List expression type, as it lends itself quite well to
	 * the concept.
	 */
	struct {
	    struct UcExpr* cond;
	    struct UcExpr* stmts;
	} if_expr;

	/*
	 * The only difference between a while expression and if expression: behavior.
	 */
	struct {
	    struct UcExpr* cond;
	    struct UcExpr* stmts;
	} while_expr;

	/*
	 * A for expression consists of an iteration target, and a list of statements that make up the body.
	 */
	struct {
	    struct UcExpr* target;
	    struct UcExpr* stmts;
	} for_expr;

	/*
	 * A yield expression is extrememly basic, as it only consists of a expression to return from the current scope.
	 */
	struct UcExpr* yield_expr;

	/*
	 * A function defintion contains a return type, arguments list, and list of statements in the body.
	 */
	struct {
	    UcArgList* args;
	    struct UcExpr* stmts;
	    enum Lexeme r_type;
	} function_decl_expr;

	/*
	 * A function call needs a target function identifier and list of arguments.
	 */
	struct {
	    char* ident;
	    struct UcExpr* args;
	} function_call_expr;
    } data;
} UcExpr;

