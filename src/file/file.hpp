#pragma once

#include <cstdlib>
#include <cassert>

#ifdef _WIN64
#include <windows.h>
#include <Fileapi.h>
#elif __unix
#include <cstdio>
#else
#pragma message("WARNING: Unknown platform, expect breakage!")
#endif

char* read_file(char* path);
