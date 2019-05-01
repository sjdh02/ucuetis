#include <cstdio>

#include "file/file.hpp"
#include "mem/arena.hpp"
#include "error/error.hpp"
#include "scanner/scanner.hpp"
#include "parser/parser.hpp"

// TODO(sam): Replace assert() in parser with custom error handling

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("provide at least one source file.\n");
	return -1;
    }
    
    auto result = read_file(argv[1]);

    UcMemArena arena;
    UcErrorStream stream;
    Tokenizer tokenizer("(defn testFn fn(a: num, b: num) => num ( (+ a b) ) )", &arena, &stream);

    Parser parser(&tokenizer, &arena, &stream);
    auto expr = parser.get_expr();

    stream.report_errors();
    
    free(result);
    return 0;
}
