#include "analyzer.h"

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream) {
    Analyzer* analyzer = amalloc(allocator, sizeof(Analyzer));
    analyzer->parser = parser;
    analyzer->allocator = allocator;
    analyzer->estream = estream;
    analyzer->symbol_table = amalloc(analyzer->allocator, sizeof(Symbol) * 10);
    analyzer->st_pos = 0;
    analyzer->st_len = 10;
    analyzer->scope_level = 0;
    
    return analyzer;
}

// NOTE(sam) @WARNING: This is just going to be messy for a while.

void analyze(Analyzer* analyzer) {
    UcExpr* expr = get_expr(analyzer->parser);
    size_t scope_level = 0;
    
    while (expr) {
        analyze_expr(analyzer, expr);
        // TODO(sam): place each expression onto an array
        expr = get_expr(analyzer->parser);
    }
}

void analyze_expr(Analyzer* analyzer, UcExpr* expr) {
    switch (expr->active) {
        case ValueExpr: {
            if (expr->data.value.active == Ident) {
                if (find_symbol(analyzer, expr->data.value.data.ident) == -1) {
                    assert(0); // error here about unknown symbol
                }
            }
            break;
        }
        
        case BoolExpr: {
            analyze_expr(analyzer, expr->data.boolean_expr.lhs);
            analyze_expr(analyzer, expr->data.boolean_expr.rhs);
            break;
        }
        
        case AssignExpr: analyze_assign_expr(analyzer, expr); break;
        
        case MathExpr: analyze_math_expr(analyzer, expr); break;
        
        case IfExpr: analyze_if_expr(analyzer, expr); break;
        
        case ForExpr: analyze_for_expr(analyzer, expr); break;
        
        case FunctionCallExpr: break;
        
        default: assert(0);
    }
}

void analyze_assign_expr(Analyzer* analyzer, UcExpr* expr) {
    if (expr->data.assign_expr.ident->active != ValueExpr ||
        expr->data.assign_expr.ident->data.value.active != Ident) {
        // push error about invalid lhs
        assert(0);	
    }
    
    analyze_expr(analyzer, expr->data.assign_expr.value);
    
    Symbol sym = {
        .ident = expr->data.assign_expr.ident->data.value.data.ident,
        .scope_level = analyzer->scope_level,
        .is_arg = false,
        .sym_expr = expr,	
    };
    
    push_symbol(analyzer, sym);
}

void analyze_math_expr(Analyzer* analyzer, UcExpr* expr) {
    bool rhsZeroOk = true;
    
    if (expr->data.math_expr.op == Div)
        rhsZeroOk = false;
    
    if (expr->data.math_expr.lhs->active == ValueExpr
        && expr->data.math_expr.lhs->data.value.active == StrLit) {
        assert(0); // cannot add strings	
    }
    
    if (expr->data.math_expr.rhs->active == ValueExpr
        && expr->data.math_expr.rhs->data.value.active == StrLit) {
        assert(0); // cannot add strings	
    }
    
    if (!rhsZeroOk
        && expr->data.math_expr.rhs->active == ValueExpr
        && expr->data.math_expr.rhs->data.value.active == NumLit
        && expr->data.math_expr.rhs->data.value.data.num_lit == 0) {
        assert(0); // cannot divide by zero
    }
    
    if (expr->data.math_expr.lhs->active == ValueExpr
        && expr->data.math_expr.lhs->data.value.active == Ident) {
        analyze_expr(analyzer, expr->data.math_expr.lhs);
    }
    
    if (expr->data.math_expr.rhs->active == ValueExpr
        && expr->data.math_expr.rhs->data.value.active == Ident) {
        analyze_expr(analyzer, expr->data.math_expr.rhs);
    }
    
}

void analyze_if_expr(Analyzer* analyzer, UcExpr* expr) {
    UcExpr** stmts_arr = expr->data.if_expr.stmts->data.list_expr;
    
    analyze_expr(analyzer, expr->data.if_expr.cond);
    while (*stmts_arr) {
        analyze_expr(analyzer, *stmts_arr);
        ++stmts_arr;
    }
}

void analyze_while_expr(Analyzer* analyzer, UcExpr* expr) {
    UcExpr** stmts_arr = expr->data.while_expr.stmts->data.list_expr;
    
    analyze_expr(analyzer, expr->data.while_expr.cond);
    while (*stmts_arr) {
        analyze_expr(analyzer, *stmts_arr);
        ++stmts_arr;
    }
}

void analyze_for_expr(Analyzer* analyzer, UcExpr* expr) {
    UcExpr** stmts_arr = expr->data.for_expr.stmts->data.list_expr;
    
    if (expr->data.for_expr.target->active != ValueExpr ||
        expr->data.for_expr.target->data.value.active != Ident) {
        assert(0); // error about invalid target for for expression
    }
    
    analyze_expr(analyzer, expr->data.for_expr.target);
    while (*stmts_arr) {
        analyze_expr(analyzer, *stmts_arr);
        ++stmts_arr;
    }
}

int find_symbol(Analyzer* analyzer, char* needle) {
    for (int i = 0; i < analyzer->st_pos; i++) {
        if (strcmp(analyzer->symbol_table[i].ident, needle) == 0)
            return i;
    }
    return -1;
}

void push_symbol(Analyzer* analyzer, Symbol symbol) {
    analyzer->symbol_table[analyzer->st_pos] = symbol;
    if (++analyzer->st_pos >= analyzer->st_len) {
        analyzer->st_len *= 2;
        analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol)* analyzer->st_len);
    }
}

void drop_expr(Analyzer* analyzer) {
    if (analyzer->st_pos == 0)
        return;
    else
        --analyzer->st_pos;
}

