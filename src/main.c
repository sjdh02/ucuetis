#include <stdio.h>

// TODO(sam): move arena types into alltypes? (BMeta)

#include "file.h"
#include "arena.h"
#include "alltypes.h"
//#include "error/error.h"
#include "scanner.h"
#include "parser.h"
//#include "analyzer/analyzer.h"

// TODO(sam): switch from a tokenizing parser to a streaming parser. Since each token
// can be stored in its char form (or char* form if its a string/ident/longer single token (in the last case (e.g. yield) we could use
// and enum)), it would be smaller and simpler to just have the parser take in each token and deal with it, I'd think. At the very
// least, the tokenizer would only need to deal with basic symbols and do a lot less parsing.

// since void* is automatically promoted to any type, there are a lot of redundant casts throughout
// the codebase. 

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("provide at least one source file.\n");
	return -1;
    }
    
    auto result = read_file(argv[1]);

    Arena* arena = init_arena();

    Tokenizer* tokenizer = init_tokenizer("(defn testFn fn(a: num, b: num) => num ( (+ a b) ) )", arena);

    Parser* parser = init_parser(tokenizer, arena);

//    Analyzer analyzer(&parser, &arena, &stream);

    afree(arena, parser);
    afree(arena, tokenizer);
    deinit_arena(arena);
    free(arena);
    free(result);    
    return 0;
}
