#include "parser.hpp"

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
	    
	case Lexeme::EOS:
	    return nullptr;
	default: assert(false); // unreachable
	}
	
	break;
    case Token::Active::NumLit:
    case Token::Active::StrLit:
    case Token::Active::Ident:
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
    default: assert(false); // unreachable
    }

    return expr;
}
