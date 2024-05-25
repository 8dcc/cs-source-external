
#ifndef UTIL_H_
#define UTIL_H_ 1

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <regex.h> /* regex_t */

#define ERR(...)                           \
    do {                                   \
        fprintf(stderr, "%s: ", __func__); \
        fprintf(stderr, __VA_ARGS__);      \
        fputc('\n', stderr);               \
    } while (0)

#define PRINT_BYTES(PTR, N)                        \
    do {                                           \
        printf("%p: ", PTR);                       \
        for (size_t i = 0; i < N; i++) {           \
            if (*((uint8_t*)(PTR) + i) < 0x10)     \
                putchar('0');                      \
            printf("%X ", *((uint8_t*)(PTR) + i)); \
        }                                          \
        putchar('\n');                             \
    } while (0)

#define GET_OFFSET(BASE, OFFSET) ((void*)((uintptr_t)BASE) + OFFSET)

/*----------------------------------------------------------------------------*/

typedef struct ModuleBounds {
    void* start;
    void* end;
    struct ModuleBounds* next;
} ModuleBounds;

/*----------------------------------------------------------------------------*/

int pidof(const char* process_name);
void readProcessMemory(pid_t pid, void* addr, void* out, size_t sz);
bool myRegex(regex_t expr, const char* str);
ModuleBounds* getModuleBounds(int pid, const char* regex);
void freeModuleBounds(ModuleBounds* bounds);
void* getModuleBaseAddress(int pid, const char* regex);

#endif /* UTIL_H_ */
