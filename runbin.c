#include <stdio.h>
#include <windows.h>

// pointer to WriteFile function
// Handle
// String to print
// Length of string
// Pointer to number of bytes written, optional can be 0
typedef void (*WriteFile_ASM)(void *, HANDLE, char *, DWORD, DWORD *);

void *load_bin(char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open file %s\n", filepath);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory and load the code into it
    void *code = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!code)
    {
        fprintf(stderr, "Error: Could not allocate memory\n");
        fclose(file);
        return NULL;
    }

    fread(code, size, 1, file);
    fclose(file);
    return code;
}

int main()
{
    char *binary_file = "C:\\Users\\janvo\\repos\\getlow\\hello.bin";

    void *code = load_bin(binary_file);

    if (code)
    {
        void *WriteFilePtr = GetProcAddress(GetModuleHandle("kernel32.dll"), "WriteFile");
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        char message[] = "Hello, from assembly!";
        DWORD bytesWritten = 0;

        // ebp-18 is WriteFilePtr
        // edp-1a t/m ebp-1e = char buffer
        printf("LFG!\n");
        ((WriteFile_ASM)(code))(WriteFilePtr, hStdOut, message, sizeof(message), 0);
    }
    else
    {
        fprintf(stderr, "Error: Could not load binary file\n");
    }

    return 0;
}