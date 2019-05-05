#include "scanner.h"

// Reserved identifiers
static const char* RESERVED[] = {
    "fn", "for", "while", "if", "else", "true", "false", "OUT", "IN", "ERROUT", "num", "str",
    "list", "it", "yield", "loop", "break", "defn",
};

Tokenizer* init_tokenizer(const char* data, Arena* allocator) {
    Tokenizer* tokenizer = (Tokenizer*)malloc(sizeof(Tokenizer));
    tokenizer->data = data;
    tokenizer->allocator = allocator;
    tokenizer->len = 0;
    tokenizer->pos = 0;
    tokenizer->line = 0;
    tokenizer->column = 0;
    tokenizer->last_len = 0;

    return tokenizer;
}

Token get_token(Tokenizer* tokenizer) {
    if (is_at_end(tokenizer)) {
	Token token;
	token.active = Lexeme;
	token.data.lexeme = EOS;
	tokenizer->last_len = 0;
	
	return token;
    }

    switch (tokenizer->data[tokenizer->pos]) {
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
	return parse_single(tokenizer);

    case '"':	    
	// skip over opening quote
	++tokenizer->pos;
	return parse_str(tokenizer);
	// skip over closing quote
	++tokenizer->pos;

    case '=':
    case '>':
    case '<':
    case '!':
	return parse_multi(tokenizer);

    case '#':
	skip_whitespace(tokenizer);
	return get_token(tokenizer);

    case ' ':
	++tokenizer->pos;
	++tokenizer->column;
	return get_token(tokenizer);

    case '\n':
	++tokenizer->pos;
	++tokenizer->line;
	tokenizer->column = 0;
	return get_token(tokenizer);

    default:
	if (is_digit(tokenizer->data[tokenizer->pos]))
	    return parse_num(tokenizer);
	else if (is_alpha(tokenizer->data[tokenizer->pos]))
	    return parse_ident(tokenizer);
	else
//	    tokenizer->stream->push_error(ErrorKind::UnknownCharacter, "tokenizer", get_pos(tokenizer));
	    ++tokenizer->pos;
	    return get_token(tokenizer);
    }
}

Token parse_ident(Tokenizer* tokenizer) {
    Token token;
    bool matched = false;
    size_t start = tokenizer->pos;
    int len = 0;
    char* buffer;

    while (tokenizer->pos < tokenizer->len && !is_delim(tokenizer->data[tokenizer->pos])) {
	++len;
	++tokenizer->pos;
    }

    buffer = (char*)amalloc(tokenizer->allocator, sizeof(char) * len + 1);
    buffer[len] = '\0';
    strncpy(buffer, tokenizer->data + start, len);

    for (size_t i = 0; i < 18; ++i) {
	if (strcmp(buffer, RESERVED[i]) == 0) {
	    matched = true;
	    afree(tokenizer->allocator, buffer);
	    token.active = Lexeme;
	    switch (i) {
	    case 0: token.data.lexeme = Fn; break;
	    case 1: token.data.lexeme = For; break;
	    case 2: token.data.lexeme = While; break;
	    case 3: token.data.lexeme = If; break;
	    case 4: token.data.lexeme = Else; break;
	    case 5: token.data.lexeme = True; break;
	    case 6: token.data.lexeme = False; break;
	    case 7: token.data.lexeme = Out; break;
	    case 8: token.data.lexeme = In; break;
	    case 9: token.data.lexeme = ErrOut; break;
	    case 10: token.data.lexeme = Num; break;
	    case 11: token.data.lexeme = Str; break;
	    case 12: token.data.lexeme = List; break;
	    case 13: token.data.lexeme = It; break;
	    case 14: token.data.lexeme = Yield; break;
	    case 15: token.data.lexeme = Loop; break;
	    case 16: token.data.lexeme = Break; break;
	    case 17: token.data.lexeme = Assign; break;
	    default: assert(false); // unreachable
	    }
	}
    }

    if (!matched) {
	token.active = Ident;
	token.data.ident = buffer;
    }
	
    tokenizer->last_len = len;
    return token;
}

Token parse_num(Tokenizer* tokenizer) {
    Token token;
    token.active = NumLit;
    size_t start = tokenizer->pos;
    int len = 0;
    char* buffer;

    while (tokenizer->pos < tokenizer->len && is_digit(tokenizer->data[tokenizer->pos])) {
	++len;
	++tokenizer->pos;
    }

    tokenizer->column += len;

    buffer = (char*)(amalloc(tokenizer->allocator, sizeof(char) * len + 1));
    buffer[len] = '\0';
    strncpy(buffer, tokenizer->data + start, len);

    token.data.num_lit = (uint64_t)strtoul(buffer, NULL, 10);
    tokenizer->last_len = len;
    afree(tokenizer->allocator, buffer);
    return token;
}

Token parse_str(Tokenizer* tokenizer) {
    Token token;
    token.active = StrLit;
    size_t start = tokenizer->pos;
    int len = 0;
    char* buffer;
    
    while (tokenizer->pos < tokenizer->len && tokenizer->data[tokenizer->pos] != '"') {
	++len;
	++tokenizer->pos;
    }

    tokenizer->column += len;
    
    buffer = (char*)amalloc(tokenizer->allocator, sizeof(char) * len + 1);
    buffer[len] = '\0';

    strncpy(buffer, tokenizer->data + start, len);

    token.data.str_lit = buffer;
    tokenizer->last_len = len;
    return token;
}

Token parse_multi(Tokenizer* tokenizer) {
    if ((tokenizer->pos + 1) >= tokenizer->len) {
	return parse_single(tokenizer);
    }
    
    Token token;
    token.active = Lexeme;
    char next = tokenizer->data[tokenizer->pos + 1];
    
    switch (tokenizer->data[tokenizer->pos]) {
    case '!':
	if (next == '=')
	    token.data.lexeme = Neq;
	else
	    token.data.lexeme = Bang;
	break;
    case '<':
	if (next == '>')
	    token.data.lexeme = Pipe;
	else if (next =='=')
	    token.data.lexeme = LtOrEq;
	else
	    token.data.lexeme = Lt;
	break;
    case '>':
	if (next == '=')
	    token.data.lexeme = GtOrEq;
	else
	    token.data.lexeme = Gt;
	break;
    case '=':
	if (next == '>')
	    token.data.lexeme = RType;
	else
	    token.data.lexeme = Eq;
	break;
    default: assert(false); // unreachable
    }

    switch (token.data.lexeme) {
    case Eq:
    case Gt:
    case Lt:
    case Bang:
	++tokenizer->pos;
	++tokenizer->column;
	tokenizer->last_len = 1;
	break;
    default:
	tokenizer->pos += 2;
	tokenizer->column += 2;
	tokenizer->last_len = 2;
	break;
    }

    return token;
}

Token parse_single(Tokenizer* tokenizer) {
    Token token;
    token.active = Lexeme;
    tokenizer->last_len = 1;

    switch (tokenizer->data[tokenizer->pos]) {
    case '(': token.data.lexeme = LParen; break;
    case ')': token.data.lexeme = RParen; break;
    case '[': token.data.lexeme = LBracket; break;
    case ']': token.data.lexeme = RBracket; break;
    case '{': token.data.lexeme = LBrace; break;
    case '}': token.data.lexeme = RBrace; break;
    case '+': token.data.lexeme = Plus; break;
    case '-': token.data.lexeme = Minus; break;
    case '*': token.data.lexeme = Mul; break;
    case '/': token.data.lexeme = Div; break;
    case ';': token.data.lexeme = Semicolon; break;
    case '|': token.data.lexeme = Bar; break;
    case ':': token.data.lexeme = Colon; break;
    case ',': token.data.lexeme = Comma; break;
    case '.': token.data.lexeme = Dot; break;
    default: assert(false); // unreachable
    }

    ++tokenizer->pos;
    return token;
}

void skip_whitespace(Tokenizer* tokenizer) {
    while (tokenizer->pos < tokenizer->len && tokenizer->data[tokenizer->pos] != '\n') {
	++tokenizer->pos;
	++tokenizer->column;
	++tokenizer->last_len;
    }
}

void step_back(Tokenizer* tokenizer) {
    tokenizer->pos -= tokenizer->last_len;
}

Token get_current_token(Tokenizer* tokenizer) {
    step_back(tokenizer);
    return get_token(tokenizer);
}

Token peek_token(Tokenizer* tokenizer) {
    Token token = get_token(tokenizer);
    step_back(tokenizer);
    return token;
}

void skip_token(Tokenizer* tokenizer) {
    get_token(tokenizer);
}

size_t get_pos(Tokenizer* tokenizer) {
    return (((size_t)tokenizer->line) << 32 | (size_t)tokenizer->column);
}

bool is_at_end(Tokenizer* tokenizer) {
    return (tokenizer->pos >= tokenizer->len);
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

