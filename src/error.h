#pragma once

#include <stdio.h>
#include "arena.h"
#include "alltypes.h"

#define MAX_ERR 64
#define MAX_ERR_LEN 128

typedef struct {
    char errors[MAX_ERR][MAX_ERR_LEN];
    size_t current_error;
} ErrorStream;

ErrorStream* init_estream();

void push_error(ErrorStream* estream, enum ErrorKind kind, char* module, size_t pos);
char* pop_error(ErrorStream* estream);
void report_errors(ErrorStream* estram);
