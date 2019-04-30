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
    Tokenizer tokenizer("(defn x 200)", &arena);

    Parser parser(&tokenizer, &arena);
    auto expr = parser.get_expr();

    printf("assignment name: %s\n", expr->data.Assign.ident->data.Value.data.Ident);
    printf("assignment value: %lu\n", expr->data.Assign.value->data.Value.data.NumLit);
    
    free(result);
    return 0;
}
