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
	token.data.Lexeme = Lexeme::EOS;
	
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
	    m_stream->push_error(ErrorKind::UnknownCharacter, "tokenizer", get_pos());
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

    buffer = static_cast<char*>(m_allocator->amalloc(sizeof(char) * len + 1));
    buffer[len] = '\0';
    strncpy(buffer, m_data + start, len);

    for (size_t i = 0; i < 18; ++i) {
	if (strcmp(buffer, RESERVED[i]) == 0) {
	    matched = true;
	    m_allocator->afree(buffer);
	    token.active = Token::Active::Lexeme;
	    switch (i) {
	    case 0: token.data.Lexeme = Lexeme::Fn; break;
	    case 1: token.data.Lexeme = Lexeme::For; break;
	    case 2: token.data.Lexeme = Lexeme::While; break;
	    case 3: token.data.Lexeme = Lexeme::If; break;
	    case 4: token.data.Lexeme = Lexeme::Else; break;
	    case 5: token.data.Lexeme = Lexeme::True; break;
	    case 6: token.data.Lexeme = Lexeme::False; break;
	    case 7: token.data.Lexeme = Lexeme::Out; break;
	    case 8: token.data.Lexeme = Lexeme::In; break;
	    case 9: token.data.Lexeme = Lexeme::ErrOut; break;
	    case 10: token.data.Lexeme = Lexeme::Num; break;
	    case 11: token.data.Lexeme = Lexeme::Str; break;
	    case 12: token.data.Lexeme = Lexeme::List; break;
	    case 13: token.data.Lexeme = Lexeme::It; break;
	    case 14: token.data.Lexeme = Lexeme::Yield; break;
	    case 15: token.data.Lexeme = Lexeme::Loop; break;
	    case 16: token.data.Lexeme = Lexeme::Break; break;
	    case 17: token.data.Lexeme = Lexeme::Assign; break;
	    default: assert(false); // unreachable
	    }
	}
    }

    if (!matched)
	token.data.Ident = buffer;
	
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

    buffer = static_cast<char*>(m_allocator->amalloc(sizeof(char) * len + 1));
    buffer[len] = '\0';
    strncpy(buffer, m_data + start, len);

    token.data.NumLit = static_cast<uint64_t>(strtoul(buffer, nullptr, 10));
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
    
    buffer = static_cast<char*>(m_allocator->amalloc(sizeof(char) * len + 1));
    buffer[len] = '\0';

    strncpy(buffer, m_data + start, len);

    token.data.StrLit = buffer;
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
	    token.data.Lexeme = Lexeme::Neq;
	else
	    token.data.Lexeme = Lexeme::Bang;
	break;
    case '<':
	if (next == '>')
	    token.data.Lexeme = Lexeme::Pipe;
	else if (next =='=')
	    token.data.Lexeme = Lexeme::LtOrEq;
	else
	    token.data.Lexeme = Lexeme::Lt;
	break;
    case '>':
	if (next == '=')
	    token.data.Lexeme = Lexeme::GtOrEq;
	else
	    token.data.Lexeme = Lexeme::Gt;
	break;
    case '=':
	if (next == '>')
	    token.data.Lexeme = Lexeme::RType;
	else
	    token.data.Lexeme = Lexeme::Eq;
	break;
    default: assert(false); // unreachable
    }

    switch (token.data.Lexeme) {
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
    case '(': token.data.Lexeme = Lexeme::LParen; break;
    case ')': token.data.Lexeme = Lexeme::RParen; break;
    case '[': token.data.Lexeme = Lexeme::LBracket; break;
    case ']': token.data.Lexeme = Lexeme::RBracket; break;
    case '{': token.data.Lexeme = Lexeme::LBrace; break;
    case '}': token.data.Lexeme = Lexeme::RBrace; break;
    case '+': token.data.Lexeme = Lexeme::Plus; break;
    case '-': token.data.Lexeme = Lexeme::Minus; break;
    case '*': token.data.Lexeme = Lexeme::Mul; break;
    case '/': token.data.Lexeme = Lexeme::Div; break;
    case ';': token.data.Lexeme = Lexeme::Semicolon; break;
    case '|': token.data.Lexeme = Lexeme::Bar; break;
    case ':': token.data.Lexeme = Lexeme::Colon; break;
    case ',': token.data.Lexeme = Lexeme::Comma; break;
    case '.': token.data.Lexeme = Lexeme::Dot; break;
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

void Tokenizer::skip_token() {
    get_next();
}

size_t Tokenizer::get_pos() {
    return ((size_t)m_line << 32) | (size_t)m_column;
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

