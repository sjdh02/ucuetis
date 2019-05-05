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
	token.active = ActiveToken.Lexeme;
	token.data.Lexeme = Lexeme.EOS;
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
	return get_next(tokenizer);

    case ' ':
	++tokenizer->pos;
	++tokenizer->column;
	return get_next(tokenizer);

    case '\n':
	++tokenizer->pos;
	++tokenizer->line;
	tokenizer->column = 0;
	return get_next(tokenizer);

    default:
	if (is_digit(tokenizer->data[tokenizer->pos]))
	    return parse_num(tokenizer);
	else if (is_alpha(tokenizer->data[tokenizer->pos]))
	    return parse_ident(tokenizer);
	else
//	    tokenizer->stream->push_error(ErrorKind::UnknownCharacter, "tokenizer", get_pos(tokenizer));
	    ++tokenizer->pos;
	    return get_next(tokenizer);
    }
}

Token parse_ident(Tokenizer* tokenizer) {
    Token token;
    token.active = ActiveToken.Ident;
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
	    tokenizer->allocator->afree(buffer);
	    token.active = ActiveToken.Lexeme;
	    switch (i) {
	    case 0: token.data.Lexeme = Lexeme.Fn; break;
	    case 1: token.data.Lexeme = Lexeme.For; break;
	    case 2: token.data.Lexeme = Lexeme.While; break;
	    case 3: token.data.Lexeme = Lexeme.If; break;
	    case 4: token.data.Lexeme = Lexeme.Else; break;
	    case 5: token.data.Lexeme = Lexeme.True; break;
	    case 6: token.data.Lexeme = Lexeme.False; break;
	    case 7: token.data.Lexeme = Lexeme.Out; break;
	    case 8: token.data.Lexeme = Lexeme.In; break;
	    case 9: token.data.Lexeme = Lexeme.ErrOut; break;
	    case 10: token.data.Lexeme = Lexeme.Num; break;
	    case 11: token.data.Lexeme = Lexeme.Str; break;
	    case 12: token.data.Lexeme = Lexeme.List; break;
	    case 13: token.data.Lexeme = Lexeme.It; break;
	    case 14: token.data.Lexeme = Lexeme.Yield; break;
	    case 15: token.data.Lexeme = Lexeme.Loop; break;
	    case 16: token.data.Lexeme = Lexeme.Break; break;
	    case 17: token.data.Lexeme = Lexeme.Assign; break;
	    default: assert(false); // unreachable
	    }
	}
    }

    if (!matched)
	token.data.Ident = buffer;
	
    tokenizer->last_len = len;
    return token;
}

Token parse_num(Tokenizer* tokenizer) {
    Token token;
    token.active = ActiveToken.NumLit;
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

    token.data.NumLit = (uint64_t)strtoul(buffer, nullptr, 10);
    tokenizer->last_len = len;
    tokenizer->allocator->afree(buffer);
    return token;
}

Token parse_str(Tokenizer* tokenizer) {
    Token token;
    token.active = ActiveToken.StrLit;
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

    token.data.StrLit = buffer;
    tokenizer->last_len = len;
    return token;
}

Token parse_multi(Tokenizer* tokenizer) {
    if ((tokenizer->pos + 1) >= tokenizer->len) {
	return parse_single(tokenizer);
    }
    
    Token token;
    token.active = ActiveToken.Lexeme;
    char next = tokenizer->data[tokenizer->pos + 1];
    
    switch (tokenizer->data[tokenizer->pos]) {
    case '!':
	if (next == '=')
	    token.data.Lexeme = Lexeme.Neq;
	else
	    token.data.Lexeme = Lexeme.Bang;
	break;
    case '<':
	if (next == '>')
	    token.data.Lexeme = Lexeme.Pipe;
	else if (next =='=')
	    token.data.Lexeme = Lexeme.LtOrEq;
	else
	    token.data.Lexeme = Lexeme.Lt;
	break;
    case '>':
	if (next == '=')
	    token.data.Lexeme = Lexeme.GtOrEq;
	else
	    token.data.Lexeme = Lexeme.Gt;
	break;
    case '=':
	if (next == '>')
	    token.data.Lexeme = Lexeme.RType;
	else
	    token.data.Lexeme = Lexeme.Eq;
	break;
    default: assert(false); // unreachable
    }

    switch (token.data.Lexeme) {
    case Lexeme.Eq:
    case Lexeme.Gt:
    case Lexeme.Lt:
    case Lexeme.Bang:
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
    token.active = ActiveToken.Lexeme;
    tokenizer->last_len = 1;

    switch (tokenizer->data[tokenizer->pos]) {
    case '(': token.data.Lexeme = Lexeme.LParen; break;
    case ')': token.data.Lexeme = Lexeme.RParen; break;
    case '[': token.data.Lexeme = Lexeme.LBracket; break;
    case ']': token.data.Lexeme = Lexeme.RBracket; break;
    case '{': token.data.Lexeme = Lexeme.LBrace; break;
    case '}': token.data.Lexeme = Lexeme.RBrace; break;
    case '+': token.data.Lexeme = Lexeme.Plus; break;
    case '-': token.data.Lexeme = Lexeme.Minus; break;
    case '*': token.data.Lexeme = Lexeme.Mul; break;
    case '/': token.data.Lexeme = Lexeme.Div; break;
    case ';': token.data.Lexeme = Lexeme.Semicolon; break;
    case '|': token.data.Lexeme = Lexeme.Bar; break;
    case ':': token.data.Lexeme = Lexeme.Colon; break;
    case ',': token.data.Lexeme = Lexeme.Comma; break;
    case '.': token.data.Lexeme = Lexeme.Dot; break;
    default: assert(false); // unreachable
    }

    ++tokenizer->pos;
    return token;
}

inline void skip_whitespace(Tokenizer* tokenizer) {
    while (tokenizer->pos < tokenizer->len && tokenizer->data[tokenizer->pos] != '\n') {
	++tokenizer->pos;
	++tokenizer->column;
	++tokenizer->last_len;
    }
}

inline void step_back(Tokenizer* tokenizer) {
    tokenizer->pos -= tokenizer->last_len;
}

inline Token get_current_token(Tokenizer* tokenizer) {
    step_back(tokenizer);
    return get_next(tokenizer);
}

Token peek_token(Tokenizer* tokenizer) {
    auto token = get_next(tokenizer);
    step_back(tokenizer);
    return token;
}

inline void skip_token(Tokenizer* tokenizer) {
    get_next(tokenizer);
}

inline size_t get_pos(Tokenizer* tokenizer) {
    return (((size_t)tokenizer->line) << 32 | (size_t)tokenizer->column);
}

inline bool is_at_end(Tokenizer* tokenizer) {
    return (tokenizer->pos >= tokenizer->len);
}

inline bool is_delim(char c) {
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

inline bool is_digit(char c) {
    return (c >= 48 && c <= 57);
}

inline bool is_alpha(char c) {
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '_');
}

