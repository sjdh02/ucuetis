#pragma once

typedef struct BMeta BMeta;
typedef struct Token Token;
typedef struct Value Value;
typedef struct UcArgList UcArgList;
typedef struct UcExpr UcExpr;

enum ErrorKind {
    UnknownCharacter,
    UnexpectedToken,
    UnexpectedEOS,    
};

enum TypeTag {
    Lexeme, NumLit,
    StrLit, Ident,
};

enum Lexeme {
    LParen, RParen,
    LBracket, RBracket,
    LBrace, RBrace,

    Lt, Gt,
    Eq, Bang,

    Plus, Minus,
    Div, Mul,

    Semicolon, Colon,
    Bar, Comma,
    Dot,

    Assign, Pipe,
    LtOrEq, GtOrEq,
    Neq, RType,

    Out, In,
    ErrOut, If,
    Else, While,
    For, Fn,
    True, False,
    It, Yield,
    Loop, Break,

    Num, Str,
    List,

    EOS,
};

enum ExprTag {
    ValueExpr, AssignExpr,
    ListExpr, BoolExpr,
    MathExpr, PipeExpr,
    IfExpr, WhileExpr,
    ForExpr, YieldExpr,
    FunctionDeclExpr, FunctionCallExpr,
    BadExpr,
};

struct BMeta {
    BMeta* next;
    size_t data_size;
    int is_free;
    int magic;
};

struct Token {
    enum TypeTag active;
    union {
	enum Lexeme lexeme;
	uint64_t num_lit;
	char* str_lit;
	char* ident;
    } data;
};

struct Value {
    enum TypeTag active;    
    union {
	enum Lexeme builtin;
	uint64_t num_lit;
	char* str_lit;
	char* ident;
    } data;
};

struct UcArgList {
    UcArgList* next;
    char* ident;
    enum Lexeme type;
};

struct UcExpr {
    enum ExprTag active;    
    union {
	Value value;

	struct {
	    UcExpr* ident;
	    UcExpr* value;
	} assign_expr;

	struct {
	    UcExpr* next;
	    UcExpr* value;
	} list_expr;

	struct {
	    enum Lexeme op;
	    UcExpr* lhs;
	    UcExpr* rhs;
	} boolean_expr;

	struct {
	    enum Lexeme op;
	    UcExpr* lhs;
	    UcExpr* rhs;
	} math_expr;
	struct {
	    UcExpr* dest;
	    UcExpr* source;
	} pipe_expr;

	struct {
	    UcExpr* cond;
	    UcExpr* stmts;
	} if_expr;

	struct {
	    UcExpr* cond;
	    UcExpr* stmts;
	} while_expr;

	struct {
	    UcExpr* target;
	    UcExpr* stmts;
	} for_expr;

	UcExpr* yield_expr;

	struct {
	    UcArgList* args;
	    UcExpr* stmts;
	    enum Lexeme r_type;
	} function_decl_expr;

	struct {
	    char* ident;
	    UcExpr* args;
	} function_call_expr;
    } data;
};

