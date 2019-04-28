#include "scanner.hpp"

Token Tokenizer::get_next() {
    if (m_pos >= m_len) {
	Token token;
	token.active = Token::Active::Symbol;
	token.data.symbol = Symbol::EOS;
	
	return token;
    }

    switch (m_data[m_pos]) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '+':
    case '-':
    case '*':
    case '/':
    case ';':
    case '|':
    case '.':
    case ',':
    case ':':
	return parse_single();

    case '"':	    
	// skip over opening quote
	++m_pos;
	return parse_str();
	// skip over closing quote
	++m_pos;

    case '=':
    case '>':
    case '<':
    case '!':
	return parse_multi();

    case '#':
	skip_whitespace();
	return get_next();

    case ' ':
	++m_pos;
	++m_column;
	return get_next();

    case '\n':
	++m_pos;
	++m_line;
	m_column = 0;
	return get_next();

    default:
	if (is_digit(m_data[m_pos]))
	    return parse_num();
	else if (is_alpha(m_data[m_pos]))
	    return parse_ident();
	else
	    printf("(tokenizer) warning: unrecognized character %c\n", m_data[m_pos]);
	    ++m_pos;
	    return get_next();
    }
}

Token Tokenizer::parse_num() {
    Token token;
    token.active = Token::Active::NumLit;
    size_t start = m_pos;
    int len = 0;
    char* buffer;

    while (m_pos < m_len && is_digit(m_data[m_pos])) {
	++len;
    }

    m_column += len;
    m_pos += len;

    // NOTE(sam): Since this buffer is very short lived, we just directly
    // call malloc rather than using the arena allocator. 
    buffer = static_cast<char*>(malloc(sizeof(char) * len + 1));
    buffer[len] = '\0';
    strncpy(buffer, m_data + start, len);

    token.data.num = static_cast<uint64_t>(strtoul(buffer, nullptr, 10));
    return token;
}

Token Tokenizer::parse_str() {
    Token token;
    token.active = Token::Active::StrLit;
    size_t start = m_pos;
    int len = 0;
    char* buffer;
    
    while (m_pos < m_len && m_data[m_pos] != '"') {
	++len;
    }

    m_column += len;
    m_pos += len;
    
    buffer = m_allocator->amalloc<char>(len + 1);
    buffer[len] = '\0';

    strncpy(buffer, m_data + start, len);

    token.data.str = buffer;
    return token;
}

Token Tokenizer::parse_multi() {
    if ((m_pos + 1) >= m_len) {
	return parse_single();
    }
    
    Token token;
    token.active = Token::Active::Symbol;
    char next = m_data[m_pos + 1];
    
    switch (m_data[m_pos]) {
    case '!':
	if (next == '=')
	    token.data.symbol = Symbol::Neq;
	else
	    token.data.symbol = Symbol::Bang;
	break;
    case '<':
	if (next == '>')
	    token.data.symbol = Symbol::Pipe;
	else if (next =='=')
	    token.data.symbol = Symbol::LtOrEq;
	else
	    token.data.symbol = Symbol::Lt;
	break;
    case '>':
	if (next == '=')
	    token.data.symbol = Symbol::GtOrEq;
	else
	    token.data.symbol = Symbol::Gt;
	break;
    case '=':
	if (next == '>')
	    token.data.symbol = Symbol::RType;
	else
	    token.data.symbol = Symbol::Eq;
	break;
    default: assert(false); // unreachable
    }

    switch (token.data.symbol) {
    case Symbol::Eq:
    case Symbol::Gt:
    case Symbol::Lt:
    case Symbol::Bang:
	++m_pos;
	++m_column;
	break;
    default:
	m_pos += 2;
	m_column += 2;
	break;
    }

    return token;
}

Token Tokenizer::parse_single() {
    Token token;
    token.active = Token::Active::Symbol;

    switch (m_data[m_pos]) {
    case '(': token.data.symbol = Symbol::LParen; break;
    case ')': token.data.symbol = Symbol::RParen; break;
    case '[': token.data.symbol = Symbol::LBracket; break;
    case ']': token.data.symbol = Symbol::RBracket; break;
    case '{': token.data.symbol = Symbol::LBrace; break;
    case '}': token.data.symbol = Symbol::LBrace; break;
    case '+': token.data.symbol = Symbol::Plus; break;
    case '-': token.data.symbol = Symbol::Minus; break;
    case '*': token.data.symbol = Symbol::Mul; break;
    case '/': token.data.symbol = Symbol::Div; break;
    case ';': token.data.symbol = Symbol::Semicolon; break;
    case '|': token.data.symbol = Symbol::Bar; break;
    case ':': token.data.symbol = Symbol::Colon; break;
    case ',': token.data.symbol = Symbol::Comma; break;
    case '.': token.data.symbol = Symbol::Dot; break;
    default: assert(false); // unreachable
    }

    ++m_pos;
    return token;
}

void Tokenizer::skip_whitespace() {
    while (m_pos < m_len && m_data[m_pos] != '\n') {
	++m_pos;
	++m_column;
	++m_last_len;
    }
}

bool is_delim(char c) {
    return (c == ';'
	    || c == '"'
	    || c == '\n'
	    || c == ' '
	    || c == ':'
	    || c == '('
	    || c == ')'
	    || c == '['
	    || c == ']'
	    || c == '<'
	    || c == '>'
	    || c == ',');
}

bool is_digit(char c) {
    return (c >= 48 && c <= 57);
}

bool is_alpha(char c) {
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 112) || c == '_');
}

