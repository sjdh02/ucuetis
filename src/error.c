#include "error.h"

void push_error(ErrorStream* estream, enum ErrorKind kind, char* module, size_t pos) {
    int line = (pos >> 32);
    int column = (pos << 32) >> 32;
        
    switch (kind) {
    case UnknownCharacter:
	snprintf(estream->errors[estream->current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unknown character at %d:%d\n",
		 module, line, column);
	break;
	
    case UnexpectedToken:
	snprintf(estream->errors[estream->current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unexpected token at %d:%d\n",
		 module, line, column);
	break;
	
    case UnexpectedEOS:
	snprintf(estream->errors[estream->current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unexpected end-of-stream at %d:%d. perhaps you forgot a closing ')'?\n",
		 module, line, column);
	break;
    }

    ++estream->current_error;
}

char* pop_error(ErrorStream* estream) {
    if (estream->current_error == 0)
	return NULL;

    --estream->current_error;
    return estream->errors[estream->current_error];
}

void report_errors(ErrorStream* estream) {
    while (estream->current_error != 0)
	printf("%s\n", pop_error(estream));
}
