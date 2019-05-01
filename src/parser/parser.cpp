#include "parser.hpp"

UcExpr* Parser::get_expr() {
    UcExpr* expr = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    Token token;

    assert(expr != nullptr);

    check_token(Token::Active::Lexeme, static_cast<uint64_t>(Lexeme::LParen), nullptr);
    token = m_tokenizer->get_next();
    
    switch (token.active) {
    case Token::Active::Lexeme:
	switch (token.data.Lexeme) {
	case Lexeme::Assign:
	    expr->active = UcExpr::Active::Assign;
	    expr->data.Assign.ident = extract_val();
	    expr->data.Assign.value = extract_val();
	    break;

	case Lexeme::Plus:
	case Lexeme::Minus:
	case Lexeme::Mul:
	case Lexeme::Div:
	    expr->active = UcExpr::Active::Math;
	    expr->data.Math.op = token.data.Lexeme;	    
	    expr->data.Math.lhs = extract_val();
	    expr->data.Math.rhs = extract_val();
	    break;

	case Lexeme::If:
	    expr->active = UcExpr::Active::If;
	    expr->data.If.cond = get_expr();
	    expr->data.If.stmts = extract_body();
	    break;
	    
	case Lexeme::While:
	    expr->active = UcExpr::Active::While;
	    expr->data.While.cond = get_expr();
	    expr->data.While.stmts = extract_body();
	    break;

	case Lexeme::For:
	    expr->active = UcExpr::Active::For;
	    check_token(Token::Active::Lexeme, static_cast<uint64_t>(Lexeme::LParen), nullptr);
	    expr->data.For.target = extract_val();
	    check_token(Token::Active::Lexeme, static_cast<uint64_t>(Lexeme::RParen), nullptr);
	    expr->data.For.stmts = extract_body();
	    break;
	    
	case Lexeme::Lt:
	case Lexeme::Gt:
	case Lexeme::Eq:
	case Lexeme::Neq:
	case Lexeme::LtOrEq:
	case Lexeme::GtOrEq:
	    expr->active = UcExpr::Active::Boolean;
	    expr->data.Boolean.op = token.data.Lexeme;
	    expr->data.Boolean.lhs = extract_val();
	    expr->data.Boolean.rhs = extract_val();
	    break;

	case Lexeme::Yield:
	    expr->active = UcExpr::Active::Yield;
	    expr->data.Yield = extract_val();
	    break;
	    
	case Lexeme::EOS:
	    return nullptr;
	default: assert(false); // unreachable
	}	
	break;

    case Token::Active::Ident:
	if (m_tokenizer->get_next().active == Token::Active::Lexeme
	    && m_tokenizer->get_current().data.Lexeme == Lexeme::LBracket) {
	    expr->active = UcExpr::Active::FunctionCall;
	    expr->data.FunctionCall.ident = token.data.Ident;
	    expr->data.FunctionCall.args = parse_function_call();
	    break;
	}
    case Token::Active::NumLit:
    case Token::Active::StrLit:
	// This should print an error instead of being marked as unreachable.
    default: assert(false); // unreachable
    }

    check_token(Token::Active::Lexeme, static_cast<uint64_t>(Lexeme::RParen), nullptr);
    
    return expr;
}

UcExpr* Parser::extract_val() {
    UcExpr* expr = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    auto token = m_tokenizer->get_next();

    switch (token.active) {
    case Token::Active::NumLit:
	expr->data.Value = Value{ Value::Active::NumLit, { token.data.NumLit } };
	break;

    case Token::Active::Ident:
	Value ident;
	ident.active = Value::Active::Ident;
	ident.data.Ident = token.data.Ident;
	expr->data.Value = ident;
	break;

    case Token::Active::StrLit:
	Value str;
	str.active = Value::Active::StrLit;
	str.data.StrLit = token.data.StrLit;
	expr->data.Value = str;
	break;

    case Token::Active::Lexeme:
	switch (token.data.Lexeme) {
	case Lexeme::Out:
	case Lexeme::In:
	case Lexeme::ErrOut:
	case Lexeme::True:
	case Lexeme::False:
	case Lexeme::It:
	    Value builtin;
	    builtin.active = Value::Active::Builtin;
	    builtin.data.Builtin = token.data.Lexeme;
	    expr->data.Value = builtin;
	    break;

	case Lexeme::LBrace:
	    expr = extract_list();
	    break;

	case Lexeme::LParen:
	    m_tokenizer->step_back();
	    expr = get_expr();
	    break;

	case Lexeme::Fn:
	    expr = parse_function_decl();
	    break;
	    // This should print an error
	default: assert(false);
	}
	   
	break;
    default: assert(false); // unreachable
    }

    return expr;
}

UcExpr* Parser::extract_list() {
    UcExpr* head = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    head->active = UcExpr::Active::List;
    UcExpr* current_node = head;
    
    while (true) {
	if (m_tokenizer->peek_token().active == Token::Active::Lexeme) {
	    if (m_tokenizer->peek_token().data.Lexeme == Lexeme::RBrace) {
		m_tokenizer->skip_token();
		break;
	    } else if (m_tokenizer->peek_token().data.Lexeme == Lexeme::Comma) {
		m_tokenizer->skip_token();
		continue;
	    } else {
		// This should print an error about an unexpected token
		assert(false);
	    }
	}
	
	current_node->data.List.value = extract_val();
	current_node->data.List.next = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
	current_node = current_node->data.List.next;
    }
    
    return head;
}

UcExpr* Parser::extract_body() {
    UcExpr* head = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    head->active = UcExpr::Active::List;
    UcExpr* current_node = head;

    while (true) {
	if (m_tokenizer->peek_token().active == Token::Active::Lexeme
	    && m_tokenizer->peek_token().data.Lexeme == Lexeme::RParen)
		break;

	current_node->data.List.value = get_expr();
	current_node->data.List.next = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
	current_node = current_node->data.List.next;
    }

    return head;
}

UcExpr* Parser::parse_function_call() {
    UcExpr* head = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    head->active = UcExpr::Active::List;
    UcExpr* current_node = head;

    while (true) {
	if (m_tokenizer->peek_token().active == Token::Active::Lexeme) {
	    if (m_tokenizer->peek_token().data.Lexeme == Lexeme::RBracket) {
		m_tokenizer->skip_token();
		break;
	    } else if (m_tokenizer->peek_token().data.Lexeme == Lexeme::Comma) {
		m_tokenizer->skip_token();
		continue;
	    }
	}
	
	
	current_node->data.List.value = extract_val();
	current_node->data.List.next = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
	current_node = current_node->data.List.next;
    }

    return head;
}

UcExpr* Parser::parse_function_decl() {
    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
    assert(m_tokenizer->get_current().data.Lexeme == Lexeme::LParen);
    UcExpr* expr = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    expr->active = UcExpr::Active::FunctionDecl;
    
    UcArgList* head = static_cast<UcArgList*>(m_allocator->amalloc(sizeof(UcArgList)));
    UcArgList* current_node = head;

    while (true) {
	if (m_tokenizer->peek_token().active == Token::Active::Lexeme) {
	    if (m_tokenizer->peek_token().data.Lexeme == Lexeme::RParen) {
		m_tokenizer->skip_token();
		break;
	    } else if (m_tokenizer->peek_token().data.Lexeme == Lexeme::Comma) {
		m_tokenizer->skip_token();
		continue;
	    }
	}

	assert(m_tokenizer->get_next().active == Token::Active::Ident);
	current_node->ident = m_tokenizer->get_current().data.Ident;

	assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
	assert(m_tokenizer->get_current().data.Lexeme == Lexeme::Colon);

	assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
	current_node->type = m_tokenizer->get_current().data.Lexeme;

	current_node->next = static_cast<UcArgList*>(m_allocator->amalloc(sizeof(UcArgList)));
	current_node = current_node->next;
    }

    expr->data.FunctionDecl.args = head;

    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
    assert(m_tokenizer->get_current().data.Lexeme == Lexeme::RType);
    
    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
    expr->data.FunctionDecl.r_type = m_tokenizer->get_current().data.Lexeme;
    
    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
    assert(m_tokenizer->get_current().data.Lexeme == Lexeme::LParen);    
    expr->data.FunctionDecl.stmts = extract_body();

    check_token(Token::Active::Lexeme, static_cast<uint64_t>(Lexeme::RParen), nullptr);    
    return expr;
}

bool Parser::check_token(Token::Active tag, uint64_t enum_or_num, char* ident_or_str) {
    auto token = m_tokenizer->get_next();
    bool cmp = false;
    
    if (token.active == tag) {
	switch (tag) {
	case Token::Active::Lexeme: cmp = (token.data.Lexeme == static_cast<Lexeme>(enum_or_num)); break;
	case Token::Active::Ident: cmp = (strcmp(token.data.Ident, ident_or_str) ? false : true); break;
	case Token::Active::NumLit: cmp = (token.data.NumLit == enum_or_num); break;
	case Token::Active::StrLit: cmp = (strcmp(token.data.StrLit, ident_or_str) ? false : true); break;
	}
    }

    if (!cmp) {
	if (token.active == Token::Active::Lexeme && token.data.Lexeme == Lexeme::EOS)
	    m_stream->push_error(ErrorKind::UnexpectedEOS, "parser", m_tokenizer->get_pos());
	else
	    m_stream->push_error(ErrorKind::UnexpectedToken, "parser", m_tokenizer->get_pos());
	
	while (true) {
	    if (m_tokenizer->peek_token().active == Token::Active::Lexeme) {
		if (m_tokenizer->peek_token().data.Lexeme == Lexeme::RParen) {
		    break;
		} else if (m_tokenizer->peek_token().data.Lexeme == Lexeme::EOS) {
		    break;
		}
	    }

	    m_tokenizer->skip_token();
	}
    }

    return cmp;
}
