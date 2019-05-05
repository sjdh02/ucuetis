#pragma once

#include <stdlib.h>
#include <assert.h>

#ifdef _WIN64
#include <windows.h>
#include <Fileapi.h>
#elif __unix
#include <stdio.h>
#else
#pragma message("WARNING: Unknown platform, expect breakage!")
#endif

char* read_file(char* path);
