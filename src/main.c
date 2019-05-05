#include <stdio.h>

#include "file.h"
#include "arena.h"
#include "alltypes.h"
//#include "error/error.h"
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
