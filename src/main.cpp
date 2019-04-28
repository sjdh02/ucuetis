#include <cstdlib>
#include <cstdio>
#include "file/file.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("provide at least one source file.\n");
	return -1;
    }
    
    auto result = read_file(argv[1]);
    printf("%s\n", result);
    
    free(result);
    return 0;
}