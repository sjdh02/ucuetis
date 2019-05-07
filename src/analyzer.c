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
    build_symbol_table(analyzer);
}

// NOTE(sam): for dealing with function call arguments, simply increment the
// scope level of the arg. that way, it can just stay in the symbol table
// without ever matching in any other scope.

void build_symbol_table(Analyzer* analyzer) {
    UcExpr* expr = get_expr(analyzer->parser);
    size_t scope_level = 0;


    while (expr != NULL) {
	if (expr->active == AssignExpr && expr->data.assign_expr.value->active == ValueExpr) {
	    Symbol sym = {.sym_expr = expr, .ident = expr->data.assign_expr.ident->data.value.data.ident, .scope_level = scope_level};
	    
	    analyzer->symbol_table[analyzer->st_pos] = sym;
	    ++analyzer->st_pos;
	} else if (expr->active == FunctionDeclExpr) {
	    ++scope_level;
	    
	    UcArgList* current_node = expr->data.function_decl_expr.args;

	    while (current_node != NULL) {
		// TODO(sam): iterate over each node of the function_decl_expr args until NULL
		// and add them to the symbol table.
	    }	  	    
	    --scope_level;
	}

	if (++analyzer->st_pos >= analyzer->st_len) {
	    // TODO(sam): write arealloc
	    break;
	}
    }
}
