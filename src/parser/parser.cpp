#include "parser.hpp"

/* 
 * TODO(sam): Go through and replace most of these assert()'s with a custom error handling solution.
 * Reporting errors > just crashing, obviously.
 * TODO(sam): A better function to check for the correct token type.
 */

UcExpr* Parser::get_expr() {
    UcExpr* expr = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
    auto token = m_tokenizer->get_next();

    assert(expr != nullptr);

    assert(token.active == Token::Active::Lexeme);
    assert(token.data.Lexeme == Lexeme::LParen);
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

	case Lexeme::For:
	    expr->active = UcExpr::Active::For;
	    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
	    assert(m_tokenizer->get_current().data.Lexeme == Lexeme::LParen);
	    expr->data.For.target = extract_val();
	    assert(m_tokenizer->get_next().active == Token::Active::Lexeme);
	    assert(m_tokenizer->get_current().data.Lexeme == Lexeme::RParen);
	    expr->data.For.statements = extract_body();
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
	
    case Token::Active::NumLit:
    case Token::Active::StrLit:
    case Token::Active::Ident:
	// This should print an error instead of being marked as unreachable.
    default: assert(false); // unreachable
    }

    token = m_tokenizer->get_next();
    assert(token.active == Token::Active::Lexeme);
    assert(token.data.Lexeme == Lexeme::RParen);
    
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
	    && m_tokenizer->peek_token().data.Lexeme == Lexeme::RParen) {
		break;
	}

	current_node->data.List.value = get_expr();
	current_node->data.List.next = static_cast<UcExpr*>(m_allocator->amalloc(sizeof(UcExpr)));
	current_node = current_node->data.List.next;
    }

    return head;
}
