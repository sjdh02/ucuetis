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

    assert(tokenizer.get_next().data.lexeme == Lexeme::LParen);
    assert(tokenizer.get_next().data.lexeme == Lexeme::Assign);
    assert(strcmp(tokenizer.get_next().data.ident, "x") == 0);
    assert(tokenizer.get_next().data.num == 200);
    assert(tokenizer.get_next().data.lexeme == Lexeme::RParen);
    assert(tokenizer.get_next().data.lexeme == Lexeme::EOS);

    UcExpr expr;
    expr.active = UcExpr::Active::Assign;
    expr.data.assign_expr.ident = "hello";

    
    free(result);    
    return 0;
}
