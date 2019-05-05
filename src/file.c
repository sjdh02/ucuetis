#include "file.h"

#ifdef _WIN64
char* read_file(char* path) {
    HANDLE file_handle;
    LARGE_INTEGER size;
    DWORD read;
    char* buffer;

    file_handle = CreateFileA(path,
			      GENERIC_READ,
			      FILE_SHARE_READ,
			      NULL,
			      OPEN_EXISTING,
			      FILE_ATTRIBUTE_NORMAL,
			      NULL);
    assert(file_handle != INVALID_HANDLE_VALUE);

    assert(GetFileSizeEx(file_handle, &size) != 0);

    buffer = malloc(sizeof(char) * (size.QuadPart + 1));

    ReadFile(file_handle, buffer, size.LowPart, &read, NULL);
    assert(read == size.LowPart);
    ReadFile(file_handle, &buffer[size.LowPart], size.HighPart, &read, NULL);
    assert(read == size.HighPart);

    CloseHandle(file_handle);
    buffer[size.QuadPart] = '\0';
    return buffer;
}
#elif __unix
char* read_file(char* path) {
    FILE* fp;
    size_t size;
    char* buffer;

    fp = fopen(path, "r");
    assert(fp != NULL);

    assert(fseek(fp, 0, SEEK_END) == 0);
    size = ftell(fp);
    rewind(fp);

    buffer = malloc(sizeof(char) * (size + 1));
    assert(fread(buffer, sizeof(char), size, fp) == size);
    assert(fclose(fp) == 0);

    buffer[size] = '\0';
    
    return buffer;
}
#endif
