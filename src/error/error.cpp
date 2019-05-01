#include "error.hpp"

void UcErrorStream::push_error(ErrorKind kind, char* module, size_t pos) {
    int line = (pos >> 32);
    int column = (pos << 32) >> 32;
        
    switch (kind) {
    case ErrorKind::UnknownCharacter:
	snprintf(m_errors[m_current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unknown character at %d:%d\n",
		 module, line, column);
	break;
	
    case ErrorKind::UnexpectedToken:
	snprintf(m_errors[m_current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unexpected token at %d:%d\n",
		 module, line, column);
	break;
	
    case ErrorKind::UnexpectedEOS:
	snprintf(m_errors[m_current_error],
		 MAX_ERR_LEN,
		 "(%s) error: unexpected end-of-stream at %d:%d. perhaps you forgot a closing ')'?\n",
		 module, line, column);
	break;
    }

    ++m_current_error;
}

char* UcErrorStream::pop_error() {
    if (m_current_error == 0)
	return "\0";

    --m_current_error;
    return m_errors[m_current_error];
}

void UcErrorStream::report_errors() {
    while (m_current_error != 0)
	printf("%s\n", pop_error());
}
