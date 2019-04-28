#include "scanner.hpp"

Token Tokenizer::get_next() {
    if (m_pos >= m_len) {
	Token token;
	token.active = Active::Symbol;
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

