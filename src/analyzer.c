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

void analyze(Analyzer* analyzer) {
    size_t scope_level = 0;
    build_symbol_table(analyzer);
}

// TODO(sam): rework this to support nested function decls.

void build_symbol_table(Analyzer* analyzer) {
    UcExpr* expr = get_expr(analyzer->parser);
    size_t scope_level = 0;

    while (expr != NULL) {
	if (expr->active == AssignExpr && expr->data.assign_expr.value->active == ValueExpr) {
	    Symbol sym;
	    sym.ident = expr->data.assign_expr.ident->data.value.data.ident;
	    sym.scope_level = scope_level;
	    sym.isArg = false;
	    sym.sym_expr = expr;
	    analyzer->symbol_table[analyzer->st_pos] = sym;
	} else if (expr->active == AssignExpr && expr->data.assign_expr.value->active == FunctionDeclExpr) {
	    UcArg* args_arr = expr->data.assign_expr.value->data.function_decl_expr.args;
	    UcExpr** stmts_arr = expr->data.assign_expr.value->data.function_decl_expr.stmts->data.list_expr;
	    ++scope_level;
	    
	    while (args_arr->type != EOS) {
		Symbol sym;
		sym.ident = args_arr->ident;
		sym.scope_level = scope_level;
		sym.isArg = true;
		sym.sym_expr = NULL;
		analyzer->symbol_table[analyzer->st_pos] = sym;

		if (++analyzer->st_pos >= analyzer->st_len) {
		    analyzer->st_len *= 2;
		    analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol) * analyzer->st_len);
		}		
		
		++args_arr;
	    }

	    while (*stmts_arr) {
		if ((*stmts_arr)->active == AssignExpr && (*stmts_arr)->data.assign_expr.value->active == ValueExpr) {
		    Symbol sym;
		    sym.ident = (*stmts_arr)->data.assign_expr.ident->data.value.data.ident;
		    sym.scope_level = scope_level;
		    sym.isArg = false;
		    sym.sym_expr = expr;
		    analyzer->symbol_table[analyzer->st_pos] = sym;
		}
		
		if (++analyzer->st_pos >= analyzer->st_len) {
		    analyzer->st_len *= 2;
		    analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol) * analyzer->st_len);
		}

		++stmts_arr;
	    }
	    
	    --scope_level;
	}

	if (++analyzer->st_pos >= analyzer->st_len) {
	    analyzer->st_len *= 2;
	    analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol) * analyzer->st_len);
	}

	expr = get_expr(analyzer->parser);
    }
}
