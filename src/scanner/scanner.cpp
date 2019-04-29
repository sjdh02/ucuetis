#include "scanner.hpp"

// Reserved identifiers
static const char* RESERVED[] = {
    "fn", "for", "while", "if", "else", "true", "false", "OUT", "IN", "ERROUT", "num", "str",
    "list", "it", "yield", "loop", "break", "defn",
};

Token Tokenizer::get_next() {
    if (m_pos >= m_len) {
	Token token;
	token.active = Token::Active::Lexeme;
	token.data.lexeme = Lexeme::EOS;
	
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

Token Tokenizer::parse_ident() {
    Token token;
    token.active = Token::Active::Ident;
    bool matched = false;
    size_t start = m_pos;
    int len = 0;
    char* buffer;

    while (m_pos < m_len && !is_delim(m_data[m_pos])) {
	++len;
	++m_pos;
    }

    buffer = m_allocator->amalloc<char*>(len + 1);
    buffer[len] = '\0';
    strncpy(buffer, m_data + start, len);

    for (size_t i = 0; i < 18; ++i) {
	if (strcmp(buffer, RESERVED[i]) == 0) {
	    matched = true;
	    m_allocator->afree(buffer);
	    token.active = Token::Active::Lexeme;
	    switch (i) {
	    case 0: token.data.lexeme = Lexeme::Fn; break;
	    case 1: token.data.lexeme = Lexeme::For; break;
	    case 2: token.data.lexeme = Lexeme::While; break;
	    case 3: token.data.lexeme = Lexeme::If; break;
	    case 4: token.data.lexeme = Lexeme::Else; break;
	    case 5: token.data.lexeme = Lexeme::True; break;
	    case 6: token.data.lexeme = Lexeme::False; break;
	    case 7: token.data.lexeme = Lexeme::Out; break;
	    case 8: token.data.lexeme = Lexeme::In; break;
	    case 9: token.data.lexeme = Lexeme::ErrOut; break;
	    case 10: token.data.lexeme = Lexeme::Num; break;
	    case 11: token.data.lexeme = Lexeme::Str; break;
	    case 12: token.data.lexeme = Lexeme::List; break;
	    case 13: token.data.lexeme = Lexeme::It; break;
	    case 14: token.data.lexeme = Lexeme::Yield; break;
	    case 15: token.data.lexeme = Lexeme::Loop; break;
	    case 16: token.data.lexeme = Lexeme::Break; break;
	    case 17: token.data.lexeme = Lexeme::Assign; break;
	    default: assert(false); // unreachable
	    }
	}
    }

    if (!matched)
	token.data.ident = buffer;
	
    m_last_len = len;
    return token;
}

Token Tokenizer::parse_num() {
    Token token;
    token.active = Token::Active::NumLit;
    size_t start = m_pos;
    int len = 0;
    char* buffer;

    while (m_pos < m_len && is_digit(m_data[m_pos])) {
	++len;
	++m_pos;
    }

    m_column += len;

    buffer = m_allocator->amalloc<char*>(len + 1);
    buffer[len] = '\0';
    strncpy(buffer, m_data + start, len);

    token.data.num = static_cast<uint64_t>(strtoul(buffer, nullptr, 10));
    m_last_len = len;
    m_allocator->afree(buffer);
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
	++m_pos;
    }

    m_column += len;
    
    buffer = m_allocator->amalloc<char*>(len + 1);
    buffer[len] = '\0';

    strncpy(buffer, m_data + start, len);

    token.data.str = buffer;
    m_last_len = len;
    return token;
}

Token Tokenizer::parse_multi() {
    if ((m_pos + 1) >= m_len) {
	return parse_single();
    }
    
    Token token;
    token.active = Token::Active::Lexeme;
    char next = m_data[m_pos + 1];
    
    switch (m_data[m_pos]) {
    case '!':
	if (next == '=')
	    token.data.lexeme = Lexeme::Neq;
	else
	    token.data.lexeme = Lexeme::Bang;
	break;
    case '<':
	if (next == '>')
	    token.data.lexeme = Lexeme::Pipe;
	else if (next =='=')
	    token.data.lexeme = Lexeme::LtOrEq;
	else
	    token.data.lexeme = Lexeme::Lt;
	break;
    case '>':
	if (next == '=')
	    token.data.lexeme = Lexeme::GtOrEq;
	else
	    token.data.lexeme = Lexeme::Gt;
	break;
    case '=':
	if (next == '>')
	    token.data.lexeme = Lexeme::RType;
	else
	    token.data.lexeme = Lexeme::Eq;
	break;
    default: assert(false); // unreachable
    }

    switch (token.data.lexeme) {
    case Lexeme::Eq:
    case Lexeme::Gt:
    case Lexeme::Lt:
    case Lexeme::Bang:
	++m_pos;
	++m_column;
	m_last_len = 1;
	break;
    default:
	m_pos += 2;
	m_column += 2;
	m_last_len = 2;
	break;
    }

    return token;
}

Token Tokenizer::parse_single() {
    Token token;
    token.active = Token::Active::Lexeme;
    m_last_len = 1;

    switch (m_data[m_pos]) {
    case '(': token.data.lexeme = Lexeme::LParen; break;
    case ')': token.data.lexeme = Lexeme::RParen; break;
    case '[': token.data.lexeme = Lexeme::LBracket; break;
    case ']': token.data.lexeme = Lexeme::RBracket; break;
    case '{': token.data.lexeme = Lexeme::LBrace; break;
    case '}': token.data.lexeme = Lexeme::LBrace; break;
    case '+': token.data.lexeme = Lexeme::Plus; break;
    case '-': token.data.lexeme = Lexeme::Minus; break;
    case '*': token.data.lexeme = Lexeme::Mul; break;
    case '/': token.data.lexeme = Lexeme::Div; break;
    case ';': token.data.lexeme = Lexeme::Semicolon; break;
    case '|': token.data.lexeme = Lexeme::Bar; break;
    case ':': token.data.lexeme = Lexeme::Colon; break;
    case ',': token.data.lexeme = Lexeme::Comma; break;
    case '.': token.data.lexeme = Lexeme::Dot; break;
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

void Tokenizer::step_back() {
    m_pos -= m_last_len;
}

Token Tokenizer::get_current() {
    step_back();
    return get_next();
}

Token Tokenizer::peek_token() {
    auto token = get_next();
    step_back();
    return token;
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
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '_');
}

