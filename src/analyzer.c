#include "analyzer.h"

Analyzer* init_analyzer(Parser* parser, Arena* allocator, ErrorStream* estream) {
    Analyzer* analyzer = amalloc(allocator, sizeof(Analyzer));
    analyzer->allocator = allocator;
    analyzer->estream = estream;
    analyzer->symbol_table = amalloc(analyzer->allocator, sizeof(Symbol) * 10);
    analyzer->pos = 0;
    
    return analyzer;
}
