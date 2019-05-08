#pragma once

#include <stdio.h>
#include "alltypes.h"
#include "arena.h"

#define MAX_ERR 64
#define MAX_ERR_LEN 128

typedef struct {
    char errors[MAX_ERR][MAX_ERR_LEN];
    size_t current_error;
} ErrorStream;

ErrorStream* init_estream(Arena* allocator);

void push_error(ErrorStream* estream, enum ErrorKind kind, char* module, size_t pos);
char* pop_error(ErrorStream* estream);
void report_errors(ErrorStream* estram);
