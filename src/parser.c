#include "parser.h"

Parser* init_parser(Tokenizer* tokenizer, Arena* allocator) {
    Parser* parser = amalloc(allocator, sizeof(Parser));
    parser->tokenizer = tokenizer;
    parser->allocator = allocator;
    return parser;
}

UcExpr* get_expr(Parser* parser) {
    if (is_at_end(parser->tokenizer))
	return NULL;
    
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    Token token;

    assert(expr != NULL);

    check_token(parser, Lexeme, (uint64_t)LParen, NULL);
    token = get_token(parser->tokenizer);
    
    switch (token.active) {
    case Lexeme:
	switch (token.data.lexeme) {
	case Assign: {
	    expr->active = AssignExpr;
	    expr->data.assign_expr.ident = extract_val(parser);
	    expr->data.assign_expr.value = extract_val(parser);
	    break;
	}

	case Plus:
	case Minus:
	case Mul:
	case Div: {
	    expr->active = MathExpr;
	    expr->data.math_expr.op = token.data.lexeme;	    
	    expr->data.math_expr.lhs = extract_val(parser);
	    expr->data.math_expr.rhs = extract_val(parser);
	    break;
	}

	case If: {
	    expr->active = IfExpr;
	    expr->data.if_expr.cond = get_expr(parser);
	    expr->data.if_expr.stmts = extract_body(parser);
	    break;
	}
	    
	case While: {
	    expr->active = WhileExpr;
	    expr->data.while_expr.cond = get_expr(parser);
	    expr->data.while_expr.stmts = extract_body(parser);
	    break;
	}

	case For: {
	    expr->active = ForExpr;
	    check_token(parser, Lexeme, (uint64_t)LParen, NULL);
	    expr->data.for_expr.target = extract_val(parser);
	    check_token(parser, Lexeme, (uint64_t)RParen, NULL);
	    expr->data.for_expr.stmts = extract_body(parser);
	    break;
	}
	    
	case Lt:
	case Gt:
	case Eq:
	case Neq:
	case LtOrEq:
	case GtOrEq: {
	    expr->active = BoolExpr;
	    expr->data.boolean_expr.op = token.data.lexeme;
	    expr->data.boolean_expr.lhs = extract_val(parser);
	    expr->data.boolean_expr.rhs = extract_val(parser);
	    break;
	}

	case Yield: {
	    expr->active = YieldExpr;
	    expr->data.yield_expr = extract_val(parser);
	    break;
	}
	    
//	case EOS: m_stream->push_error(ErrorKind::UnexpectedEOS, "parser", m_tokenizer->get_pos()); break;
	case EOS:
	default: assert(false); // unreachable
	}	
	break;

    case Ident:
	if (get_token(parser->tokenizer).active == Lexeme
	    && get_current_token(parser->tokenizer).data.lexeme == LBracket) {
	    expr->active = FunctionCallExpr;
	    expr->data.function_call_expr.ident = token.data.ident;
	    expr->data.function_call_expr.args = parse_function_call(parser);
	    break;
	}
    case NumLit:
    case StrLit:
	// This should print an error instead of being marked as unreachable.
    default: assert(false); // unreachable
    }

    check_token(parser, Lexeme, (uint64_t)RParen, NULL);
    
    return expr;
}

UcExpr* extract_val(Parser* parser) {
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    expr->active = ValueExpr;
    
    Token token = get_token(parser->tokenizer);
    
    switch (token.active) {
    case NumLit: {
	Value num;
	num.active = NumLit;
	num.data.num_lit = token.data.num_lit;
	expr->data.value = num;
	break;
    }

    case Ident: {
	Value ident;
	ident.active = Ident;
	ident.data.ident = token.data.ident;
	expr->data.value = ident;
	break;
    }

    case StrLit: {
	Value str;
	str.active = StrLit;
	str.data.str_lit = token.data.str_lit;
	expr->data.value = str;
	break;
    }

    case Lexeme:
	switch (token.data.lexeme) {
	case Out:
	case In:
	case ErrOut:
	case True:
	case False:
	case It: {
	    Value builtin;
	    builtin.active = Lexeme;
	    builtin.data.builtin = token.data.lexeme;
	    expr->data.value = builtin;
	    break;
	}

// NOTE(sam): We have to call afree in the following cases because the functions
// that are called return pre-allocated expressions, negating the need for
// the expression allocated in this function.	    
	case LBrace: {
	    afree(parser->allocator, expr);
	    expr = extract_list(parser);
	    break;
	}

	case LParen: {
	    step_back(parser->tokenizer);
	    afree(parser->allocator, expr);
	    expr = get_expr(parser);
	    break;
	}

	case Fn: {
	    afree(parser->allocator, expr);
	    expr = parse_function_decl(parser);
	    break;
	}
	    // This should print an error
	default: assert(false);
	}
	   
	break;
	
    default: assert(false); // unreachable
    }

    return expr;
}

UcExpr* extract_list(Parser* parser) {
    UcExpr* head = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    head->active = ListExpr;

    UcExpr* current_node = head;
    
    while (true) {
	if (peek_token(parser->tokenizer).active == Lexeme) {
	    if (peek_token(parser->tokenizer).data.lexeme == RBrace) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.lexeme == Comma) {
		skip_token(parser->tokenizer);
		continue;
	    } else {
		// This should print an error about an unexpected token
		assert(false);
	    }
	}
	
	current_node->data.list_expr.value = extract_val(parser);
	current_node->data.list_expr.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.list_expr.next;
    }
    
    return head;
}

UcExpr* extract_body(Parser* parser) {
    UcExpr* head = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    head->active = ListExpr;
    
    UcExpr* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == Lexeme
	    && peek_token(parser->tokenizer).data.lexeme == RParen)
		break;

	current_node->data.list_expr.value = get_expr(parser);
	current_node->data.list_expr.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.list_expr.next;
    }

    return head;
}

UcExpr* parse_function_call(Parser* parser) {
    UcExpr* head = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    head->active = ListExpr;
    
    UcExpr* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == Lexeme) {
	    if (peek_token(parser->tokenizer).data.lexeme == RBracket) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.lexeme == Comma) {
		skip_token(parser->tokenizer);
		continue;
	    }
	}
	
	
	current_node->data.list_expr.value = extract_val(parser);
	current_node->data.list_expr.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.list_expr.next;
    }

    return head;
}

UcExpr* parse_function_decl(Parser* parser) {
    assert(get_token(parser->tokenizer).active == Lexeme);
    assert(get_current_token(parser->tokenizer).data.lexeme == LParen);
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    expr->active = FunctionDeclExpr;
    
    UcArgList* head = (UcArgList*)amalloc(parser->allocator, sizeof(UcArgList));
    UcArgList* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == Lexeme) {
	    if (peek_token(parser->tokenizer).data.lexeme == RParen) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.lexeme == Comma) {
		skip_token(parser->tokenizer);
		continue;
	    }
	}

	assert(get_token(parser->tokenizer).active == Ident);
	current_node->ident = get_current_token(parser->tokenizer).data.ident;

	assert(get_token(parser->tokenizer).active == Lexeme);
	assert(get_current_token(parser->tokenizer).data.lexeme == Colon);

	assert(get_token(parser->tokenizer).active == Lexeme);
	current_node->type = get_current_token(parser->tokenizer).data.lexeme;

	current_node->next = (UcArgList*)amalloc(parser->allocator, sizeof(UcArgList));
	current_node = current_node->next;
    }

    expr->data.function_decl_expr.args = head;

    assert(get_token(parser->tokenizer).active == Lexeme);
    assert(get_current_token(parser->tokenizer).data.lexeme == RType);
    
    assert(get_token(parser->tokenizer).active == Lexeme);
    expr->data.function_decl_expr.r_type = get_current_token(parser->tokenizer).data.lexeme;
    
    assert(get_token(parser->tokenizer).active == Lexeme);
    assert(get_current_token(parser->tokenizer).data.lexeme == LParen);    
    expr->data.function_decl_expr.stmts = extract_body(parser);

    check_token(parser, Lexeme, (uint64_t)RParen, NULL);    
    return expr;
}

bool check_token(Parser* parser, enum TokenTag tag, uint64_t enum_or_num, char* ident_or_str) {
    Token token = get_token(parser->tokenizer);
    bool cmp = false;
    
    if (token.active == tag) {
	switch (tag) {
	case Lexeme: cmp = (token.data.lexeme == (enum Lexeme)enum_or_num); break;
	case Ident: cmp = (strcmp(token.data.ident, ident_or_str) ? false : true); break;
	case NumLit: cmp = (token.data.num_lit == enum_or_num); break;
	case StrLit: cmp = (strcmp(token.data.str_lit, ident_or_str) ? false : true); break;
	}
    }

    if (!cmp) {
	/* Pending error stream rework
	if (token.active == Lexeme && token.data.lexeme == EOS)
	    m_stream->push_error(ErrorKind::UnexpectedEOS, "parser", m_tokenizer->get_pos(parser));
	else
	    m_stream->push_error(ErrorKind::UnexpectedToken, "parser", m_tokenizer->get_pos(parser));
	*/
	while (true) {
	    if (peek_token(parser->tokenizer).active == Lexeme) {
		if (peek_token(parser->tokenizer).data.lexeme == RParen) {
		    break;
		} else if (peek_token(parser->tokenizer).data.lexeme == EOS) {
		    break;
		}
	    }

	    skip_token(parser->tokenizer);
	}
    }

    return cmp;
}
