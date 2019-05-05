#include "parser.h"

UcExpr* get_expr(Parser* parser) {
    if (m_tokenizer->is_at_end(parser))
	return nullptr;
    
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    Token token;

    assert(expr != NULL);

    check_token(ActiveToken.Lexeme, (uint64_t)Lexeme.LParen, NULL);
    token = get_token(parser->tokenizer);
    
    switch (token.active) {
    case ActiveToken.Lexeme:
	switch (token.data.Lexeme) {
	case Lexeme.Assign:
	    expr->active = ActiveExpr.Assign;
	    expr->data.Assign.ident = extract_val(parser);
	    expr->data.Assign.value = extract_val(parser);
	    break;

	case Lexeme.Plus:
	case Lexeme.Minus:
	case Lexeme.Mul:
	case Lexeme.Div:
	    expr->active = ActiveExpr.Math;
	    expr->data.Math.op = token.data.Lexeme;	    
	    expr->data.Math.lhs = extract_val(parser);
	    expr->data.Math.rhs = extract_val(parser);
	    break;

	case Lexeme.If:
	    expr->active = ActiveExpr.If;
	    expr->data.If.cond = get_expr(parser);
	    expr->data.If.stmts = extract_body(parser);
	    break;
	    
	case Lexeme.While:
	    expr->active = ActiveExpr.While;
	    expr->data.While.cond = get_expr(parser);
	    expr->data.While.stmts = extract_body(parser);
	    break;

	case Lexeme.For:
	    expr->active = ActiveExpr.For;
	    check_token(ActiveToken.Lexeme, (uint64_t)Lexeme.LParen, nullptr);
	    expr->data.For.target = extract_val(parser);
	    check_token(ActiveToken.Lexeme, (uint64_t)Lexeme.RParen, nullptr);
	    expr->data.For.stmts = extract_body(parser);
	    break;
	    
	case Lexeme.Lt:
	case Lexeme.Gt:
	case Lexeme.Eq:
	case Lexeme.Neq:
	case Lexeme.LtOrEq:
	case Lexeme.GtOrEq:
	    expr->active = ActiveExpr.Boolean;
	    expr->data.Boolean.op = token.data.Lexeme;
	    expr->data.Boolean.lhs = extract_val(parser);
	    expr->data.Boolean.rhs = extract_val(parser);
	    break;

	case Lexeme.Yield:
	    expr->active = ActiveExpr.Yield;
	    expr->data.Yield = extract_val(parser);
	    break;
	    
//	case Lexeme.EOS: m_stream->push_error(ErrorKind::UnexpectedEOS, "parser", m_tokenizer->get_pos()); break;
	case Lexeme.EOS:
	default: assert(false); // unreachable
	}	
	break;

    case ActiveToken.Ident:
	if (get_token(parser->tokenizer).active == ActiveToken.Lexeme
	    && get_current_token(parser->tokenizer).data.Lexeme == Lexeme.LBracket) {
	    expr->active = ActiveExpr.FunctionCall;
	    expr->data.FunctionCall.ident = token.data.Ident;
	    expr->data.FunctionCall.args = parse_function_call(parser);
	    break;
	}
    case ActiveToken.NumLit:
    case ActiveToken.StrLit:
	// This should print an error instead of being marked as unreachable.
    default: assert(false); // unreachable
    }

    check_token(ActiveToken.Lexeme, (uint64_t)Lexeme.RParen, nullptr);
    
    return expr;
}

UcExpr* extract_val(Parser* parser) {
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    auto token = get_token(parser->tokenizer);

    switch (token.active) {
    case ActiveToken.NumLit:
	expr->data.Value = Value{ ActiveValue.NumLit, { token.data.NumLit } };
	break;

    case ActiveToken.Ident:
	Value ident;
	ident.active = ActiveValue.Ident;
	ident.data.Ident = token.data.Ident;
	expr->data.Value = ident;
	break;

    case ActiveToken.StrLit:
	Value str;
	str.active = ActiveValue.StrLit;
	str.data.StrLit = token.data.StrLit;
	expr->data.Value = str;
	break;

    case ActiveToken.Lexeme:
	switch (token.data.Lexeme) {
	case Lexeme.Out:
	case Lexeme.In:
	case Lexeme.ErrOut:
	case Lexeme.True:
	case Lexeme.False:
	case Lexeme.It:
	    Value builtin;
	    builtin.active = ActiveValue.Builtin;
	    builtin.data.Builtin = token.data.Lexeme;
	    expr->data.Value = builtin;
	    break;

	case Lexeme.LBrace:
	    expr = extract_list(parser);
	    break;

	case Lexeme.LParen:
	    step_back(parser->tokenizer);
	    expr = get_expr(parser);
	    break;

	case Lexeme.Fn:
	    expr = parse_function_decl(parser);
	    break;
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
    head->active = ActiveExpr.List;
    UcExpr* current_node = head;
    
    while (true) {
	if (peek_token(parser->tokenizer).active == ActiveToken.Lexeme) {
	    if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.RBrace) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.Comma) {
		skip_token(parser->tokenizer);
		continue;
	    } else {
		// This should print an error about an unexpected token
		assert(false);
	    }
	}
	
	current_node->data.List.value = extract_val(parser);
	current_node->data.List.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.List.next;
    }
    
    return head;
}

UcExpr* extract_body(Parser* parser) {
    UcExpr* head = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    head->active = ActiveExpr.List;
    UcExpr* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == ActiveToken.Lexeme
	    && peek_token(parser->tokenizer).data.Lexeme == Lexeme.RParen)
		break;

	current_node->data.List.value = get_expr(parser);
	current_node->data.List.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.List.next;
    }

    return head;
}

UcExpr* parse_function_call(Parser* parser) {
    UcExpr* head = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    head->active = ActiveExpr.List;
    UcExpr* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == ActiveToken.Lexeme) {
	    if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.RBracket) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.Comma) {
		skip_token(parser->tokenizer);
		continue;
	    }
	}
	
	
	current_node->data.List.value = extract_val(parser);
	current_node->data.List.next = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
	current_node = current_node->data.List.next;
    }

    return head;
}

UcExpr* parse_function_decl(Parser* parser) {
    assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
    assert(get_current_token(parser->tokenizer).data.Lexeme == Lexeme.LParen);
    UcExpr* expr = (UcExpr*)amalloc(parser->allocator, sizeof(UcExpr));
    expr->active = ActiveExpr.FunctionDecl;
    
    UcArgList* head = (UcArgList*)amalloc(parser->allocator, sizeof(UcArgList));
    UcArgList* current_node = head;

    while (true) {
	if (peek_token(parser->tokenizer).active == ActiveToken.Lexeme) {
	    if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.RParen) {
		skip_token(parser->tokenizer);
		break;
	    } else if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.Comma) {
		skip_token(parser->tokenizer);
		continue;
	    }
	}

	assert(get_token(parser->tokenizer).active == ActiveToken.Ident);
	current_node->ident = get_current_token(parser->tokenizer).data.Ident;

	assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
	assert(get_current_token(parser->tokenizer).data.Lexeme == Lexeme.Colon);

	assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
	current_node->type = get_current_token(parser->tokenizer).data.Lexeme;

	current_node->next = (UcArgList*)amalloc(parser->allocator, sizeof(UcArgList));
	current_node = current_node->next;
    }

    expr->data.FunctionDecl.args = head;

    assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
    assert(get_current_token(parser->tokenizer).data.Lexeme == Lexeme.RType);
    
    assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
    expr->data.FunctionDecl.r_type = get_current_token(parser->tokenizer).data.Lexeme;
    
    assert(get_token(parser->tokenizer).active == ActiveToken.Lexeme);
    assert(get_current_token(parser->tokenizer).data.Lexeme == Lexeme.LParen);    
    expr->data.FunctionDecl.stmts = extract_body(parser);

    check_token(ActiveToken.Lexeme, (uint64_t)Lexeme.RParen, nullptr);    
    return expr;
}

bool check_token(Token::Active tag, uint64_t enum_or_num, char* ident_or_str) {
    auto token = get_token(parser->tokenizer);
    bool cmp = false;
    
    if (token.active == tag) {
	switch (tag) {
	case ActiveToken.Lexeme: cmp = (token.data.Lexeme == (Lexeme)enum_or_num); break;
	case ActiveToken.Ident: cmp = (strcmp(token.data.Ident, ident_or_str) ? false : true); break;
	case ActiveToken.NumLit: cmp = (token.data.NumLit == enum_or_num); break;
	case ActiveToken.StrLit: cmp = (strcmp(token.data.StrLit, ident_or_str) ? false : true); break;
	}
    }

    if (!cmp) {
	/* Pending error stream rework
	if (token.active == ActiveToken.Lexeme && token.data.Lexeme == Lexeme.EOS)
	    m_stream->push_error(ErrorKind::UnexpectedEOS, "parser", m_tokenizer->get_pos(parser));
	else
	    m_stream->push_error(ErrorKind::UnexpectedToken, "parser", m_tokenizer->get_pos(parser));
	*/
	while (true) {
	    if (peek_token(parser->tokenizer).active == ActiveToken.Lexeme) {
		if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.RParen) {
		    break;
		} else if (peek_token(parser->tokenizer).data.Lexeme == Lexeme.EOS) {
		    break;
		}
	    }

	    skip_token(parser->tokenizer);
	}
    }

    return cmp;
}
