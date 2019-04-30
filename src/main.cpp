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
    Tokenizer tokenizer("(defn testFn fn(a: num, b: num) => num ( (+ a b) ) )", &arena);

    Parser parser(&tokenizer, &arena);
    auto expr = parser.get_expr();

    free(result);
    return 0;
}
