#include <stdio.h>

#include "alltypes.h"
#include "file.h"
#include "error.h"
#include "arena.h"
#include "scanner.h"
#include "parser.h"
//#include "analyzer/analyzer.h"

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("provide at least one source file.\n");
	return -1;
    }
		
    char* result = read_file(argv[1]);
    Arena* arena = init_arena();
    ErrorStream* estream = init_estream(arena);
    // NOTE(sam): This is handing out two pointers to the error stream, to both the tokenizer and parser.
    // This should be fine for single-threaded code, but if multithreading is added, push_error and pop_error
    // would need to be mutexed.
    Tokenizer* tokenizer = init_tokenizer("(defn testFn fn(a: num, b: num) => num ( (+ a b) ) )", arena, estream);
    Parser* parser = init_parser(tokenizer, arena, estream);

    UcExpr* expr = get_expr(parser);
    
    report_errors(estream);
    
    afree(arena, parser);
    afree(arena, tokenizer);
    afree(arena, estream);
    deinit_arena(arena);
    free(arena);
    free(result);    
    return 0;
}
