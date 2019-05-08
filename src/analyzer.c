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

// NOTE(sam): for dealing with function call arguments, simply increment the
// scope level of the arg. that way, it can just stay in the symbol table
// without ever matching in any other scope.

// TODO(sam): rework this to support nested function decls.

void build_symbol_table(Analyzer* analyzer) {
    UcExpr* expr = get_expr(analyzer->parser);
    size_t scope_level = 0;

    while (expr) {
	if (expr->active == AssignExpr && expr->data.assign_expr.value->active == ValueExpr) {
	    Symbol sym;
	    sym.ident = expr->data.assign_expr.ident->data.value.data.ident;
	    sym.scope_level = scope_level;
	    sym.isArg = false;
	    sym.sym_expr = expr;
	    analyzer->symbol_table[analyzer->st_pos] = sym;
	} else if (expr->active == FunctionDeclExpr) {
	    UcArgList* current_arg = expr->data.function_decl_expr.args;
	    UcExpr* current_stmt = expr->data.function_decl_expr.stmts;	    
	    ++scope_level;

	    while (current_arg) {
		Symbol sym;
		sym.ident = current_arg->ident;
		sym.scope_level = scope_level;
		sym.isArg = true;
		sym.sym_expr = NULL;
		analyzer->symbol_table[analyzer->st_pos] = sym;		

		if (++analyzer->st_pos >= analyzer->st_len) {
		    analyzer->st_len *= 2;
		    analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol) * analyzer->st_len);
		}

		current_arg = current_arg->next;
	    }
	    
	    while (current_stmt) {
		if (expr->active == AssignExpr && expr->data.assign_expr.value->active == ValueExpr) {
		    Symbol sym;
		    sym.ident = expr->data.assign_expr.ident->data.value.data.ident;
		    sym.scope_level = scope_level;
		    sym.isArg = false;
		    sym.sym_expr = expr;
		    analyzer->symbol_table[analyzer->st_pos] = sym;
		}
		
		if (++analyzer->st_pos >= analyzer->st_len) {
		    analyzer->st_len *= 2;
		    analyzer->symbol_table = arealloc(analyzer->allocator, analyzer->symbol_table, sizeof(Symbol) * analyzer->st_len);
		}

		current_stmt = current_stmt->data.list_expr.next;
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
