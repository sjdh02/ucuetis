#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct BMeta BMeta;
typedef struct Token Token;
typedef struct Value Value;
typedef struct UcArg UcArg;
typedef struct UcExpr UcExpr;
typedef struct Symbol Symbol;

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
    
    Assign, LtOrEq,
    GtOrEq, Neq,
    RType,
    
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
    ValueExpr, ListExpr,
    BoolExpr, AssignExpr,
    MathExpr, IfExpr,
    WhileExpr, ForExpr,
    YieldExpr, FunctionDeclExpr,
    FunctionCallExpr,
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

struct UcArg {
    char* ident;
    enum Lexeme type;
};

// TODO(sam): change all ambiguous names to
// rhs and lhs.
struct UcExpr {
    enum ExprTag active;    
    union {
        Value value;
        
        UcExpr** list_expr;
        
        struct {
            enum Lexeme op;
            UcExpr* lhs;
            UcExpr* rhs;
        } boolean_expr;
        
        struct {
            UcExpr* ident;
            UcExpr* value;
        } assign_expr;
        
        struct {
            enum Lexeme op;
            UcExpr* lhs;
            UcExpr* rhs;
        } math_expr;
        
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
            UcArg* args;
            UcExpr* stmts;
            enum Lexeme r_type;
        } function_decl_expr;
        
        struct {
            char* ident;
            UcExpr* args;
        } function_call_expr;
    } data;
};

struct Symbol {
    char* ident;
    size_t scope_level;
    bool is_arg;
    UcExpr* sym_expr;
};
