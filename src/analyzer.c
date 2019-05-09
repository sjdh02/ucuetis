#include "analyzer.h"

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream) {
    Analyzer* analyzer = amalloc(allocator, sizeof(Analyzer));
    analyzer->parser = parser;
    analyzer->allocator = allocator;
    analyzer->estream = estream;
    analyzer->symbol_table = amalloc(analyzer->allocator, sizeof(Symbol) * 10);
    analyzer->st_pos = 0;
    analyzer->st_len = 10;
    
    return analyzer;
}

// NOTE(sam): This should really be structured much like the parser and extract_val.
// We can have one global function which does the analysis, and calls out to functions
// such as analyze_expr to do analysis of individual expression types.

// Right now, the biggest question about this design deals with the potential
// rhs values of expressions. E.g., in an assign, we currently cannot analyze
// the rhs if its a function call for example. in a similar vein, many
// nested expression won't work. Ideally, from analyze_assign_expr,
// we would be able to call a function to analyze function exprs,
// which could recursively call whatever it needed to in order to
// analyze. Again, see the above NOTE about something to reference when doing
// this.

void analyze(Analyzer* analyzer) {
    UcExpr* expr = get_expr(analyzer->parser);
    size_t scope_level = 0;

    while (expr) {
	switch (expr->active) {
	case AssignExpr: analyze_assign_expr(analyzer, expr); break;
	default: break;
	}
	
	expr = get_expr(analyzer->parser);
    }
}

void analyze_assign_expr(Analyzer* analyzer, UcExpr* expr) {
    size_t scope_level = 0;
    
    if (expr->data.assign_expr.value->active == ValueExpr) {
	Symbol sym;
	sym.ident = expr->data.assign_expr.ident->data.value.data.ident;
	sym.scope_level = scope_level;
	sym.is_arg = false;
	sym.sym_expr = expr;
	analyzer->symbol_table[analyzer->st_pos] = sym;
    } else if (expr->data.assign_expr.value->active == FunctionDeclExpr) {
	UcArg* args_arr = expr->data.assign_expr.value->data.function_decl_expr.args;
	UcExpr** stmts_arr = expr->data.assign_expr.value->data.function_decl_expr.stmts->data.list_expr;
	Symbol sym = {
	    .ident = expr->data.assign_expr.ident->data.value.data.ident,
	    .scope_level = scope_level,
	    .is_arg = false,
	    .sym_expr = expr
	};
	
	push_symbol(analyzer, sym);
	++scope_level;
	
	while (args_arr->type != EOS) {
	    Symbol sym = {
		.ident = args_arr->ident,
		.scope_level = scope_level,
		.is_arg = true,
		.sym_expr = NULL,
	    };
	    push_symbol(analyzer, sym);	    
	    ++args_arr;
	}

	while (*stmts_arr) {
	    if ((*stmts_arr)->active == AssignExpr && (*stmts_arr)->data.assign_expr.value->active == ValueExpr) {
		Symbol sym = {
		    .ident = (*stmts_arr)->data.assign_expr.ident->data.value.data.ident,
		    .scope_level = scope_level,
		    .is_arg = false,
		    .sym_expr = expr,
		};
		push_symbol(analyzer, sym);
	    }
		
	    ++stmts_arr;
	}
	    
	--scope_level;
    }
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

