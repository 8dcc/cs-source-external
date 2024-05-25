
#ifndef SDK_H_
#define SDK_H_ 1

#include <stdint.h>
#include "globals.h"
#include "util.h"

/*----------------------------------------------------------------------------*/

typedef struct vec2_t {
    float x, y;
} vec2_t;

/*----------------------------------------------------------------------------*/

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

static inline bool vecIsZero(vec3_t x) {
    return x.x == 0.f && x.y == 0.f && x.z == 0.f;
}

/*----------------------------------------------------------------------------*/

#define MAX_NAME    256
#define MAX_PLAYERS 64

typedef void Player;

static inline char* getPlayerName(Player* x, char* name) {
    const int offset = 0x10;
    void* addr       = (void*)((uintptr_t)x + offset);

    readProcessMemory(g_pid, addr, name, MAX_NAME);
    return name;
}

static inline int getPlayerHealth(Player* x) {
    const int offset = 0x34;
    void* addr       = (void*)((uintptr_t)x + offset);

    int ret;
    readProcessMemory(g_pid, addr, &ret, sizeof(ret));
    return ret;
}

static inline vec3_t getPlayerPos(Player* x) {
    const int offset = 0x38;
    void* addr       = (void*)((uintptr_t)x + offset);

    vec3_t ret;
    readProcessMemory(g_pid, addr, &ret, sizeof(ret));
    return ret;
}

#endif /* SDK_H_ */
