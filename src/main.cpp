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
    Tokenizer tokenizer("(if (< x 200) (+ x 200))", &arena);

    Parser parser(&tokenizer, &arena);
    auto expr = parser.get_expr();

    printf("target: %d\n", expr->data.If.statements->data.List.value->data.Math.op);
    printf("target: %s\n", expr->data.If.statements->data.List.value->data.Math.lhs->data.Value.data.Ident);
    printf("target: %d\n", expr->data.If.statements->data.List.value->data.Math.rhs->data.Value.data.NumLit);
    
    free(result);
    return 0;
}
