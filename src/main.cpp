#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>

#include "file/file.hpp"
#include "mem/arena.hpp"
#include "scanner/scanner.hpp"
#include "parser/parser.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("provide at least one source file.\n");
	return -1;
    }
    
    auto result = read_file(argv[1]);

    UcMemArena arena;
    Tokenizer tokenizer("(testFn[1, 2, 3, 4, 5])", &arena);

    Parser parser(&tokenizer, &arena);
    auto expr = parser.get_expr();

    printf("%s\n", expr->data.FunctionCall.ident);
    printf("%d\n", expr->data.FunctionCall.args->data.List.value->data.Value.data.NumLit);
    
    free(result);
    return 0;
}
