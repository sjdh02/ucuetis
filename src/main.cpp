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
    Tokenizer tokenizer("(for (items) (+ it 200) (+ it 300))", &arena);

    Parser parser(&tokenizer, &arena);
    auto expr = parser.get_expr();

    printf("target: %s\n", expr->data.For.target->data.Value.data.Ident);
    printf("expression: %d\n", expr->data.For.statements->data.List.next->data.List.value->data.Math.rhs->data.Value.data.NumLit);
    
    free(result);
    return 0;
}
