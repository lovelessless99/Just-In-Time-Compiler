#include <stdio.h> // printf
#include <string.h> // memcpy
#include <sys/mman.h> // mmap, munmap

int main () {
// Hexadecimal x86_64 machine code for: int mul (int a, int b) { return a * b; }
unsigned char code [] = {
        0x55, // push rbp
        0x48, 0x89, 0xe5, // mov rbp, rsp
        0x89, 0x7d, 0xfc, // mov DWORD PTR [rbp-0x4],edi
        0x89, 0x75, 0xf8, // mov DWORD PTR [rbp-0x8],esi
        0x8b, 0x75, 0xfc, // mov esi,DWORD PTR [rbp-04x]
        0x0f, 0xaf, 0x75, 0xf8, // imul esi,DWORD PTR [rbp-0x8]
        0x89, 0xf0, // mov eax,esi
        0x5d, // pop rbp
        0xc3 // ret
};

        // allocate executable memory via sys call
        void* mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                        MAP_ANON | MAP_PRIVATE, -1, 0);

        // copy runtime code into allocated memory
        memcpy(mem, code, sizeof(code));

        // typecast allocated memory to a function pointer
        int (*func) () = mem;

        // call function pointer
        printf("%d * %d = %d\n", 5, 11, func(5, 11));

        // Free up allocated memory
        munmap(mem, sizeof(code));
}

