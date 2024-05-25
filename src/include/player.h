
#ifndef PLAYER_H_
#define PLAYER_H_ 1

#include "sdk.h"
#include "globals.h"

#define PLAYER_GETTER(TYPE, NAME, OFFSET)                   \
    static inline TYPE getPlayer##NAME(Player* x) {         \
        if (x == NULL) {                                    \
            return (TYPE)0;                                 \
        }                                                   \
        void* addr = (void*)((uintptr_t)x + OFFSET);        \
        TYPE ret;                                           \
        readProcessMemory(g_pid, addr, &ret, sizeof(TYPE)); \
        return ret;                                         \
    }

/*----------------------------------------------------------------------------*/

static inline char* getPlayerName(Player* x, char* name) {
    if (x == NULL)
        return NULL;

    const int offset = 0x10;
    void* addr       = (void*)((uintptr_t)x + offset);

    readProcessMemory(g_pid, addr, name, MAX_NAME);
    return name;
}

static inline vec3_t getPlayerPos(Player* x) {
    if (x == NULL)
        return VEC_ZERO;

    const int offset = 0x38;
    void* addr       = (void*)((uintptr_t)x + offset);

    vec3_t ret;
    readProcessMemory(g_pid, addr, &ret, sizeof(vec3_t));
    return ret;
}

PLAYER_GETTER(int, Team, 0x30);
PLAYER_GETTER(int, Health, 0x34);

#endif /* PLAYER_H_ */
