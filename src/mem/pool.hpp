#pragma once

#include <cstdlib>
#include <cassert>

/*
 * Notes: We'll need to keep a linked list of metadata with information about where blocks start and end 
 * to facilitate memory reuse. Since the memory will just be stored as a unsigned char* memory, we can
 * just store the offset for where a block starts and use the size of whatever it allocated to mark
 * where it ends. We can then check over this list to see if a block can be reused. We could also
 * use an array to keep track of each block, but a linked list has better insertion time for inserting
 * new blocks.
 */

class UcMemPool {
public:
    UcMemPool();
    ~UcMemPool();
};
