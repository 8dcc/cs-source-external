
#ifndef SDK_H_
#define SDK_H_ 1

#include <stdint.h>
#include "util.h"

/*----------------------------------------------------------------------------*/

typedef struct vec2_t {
    float x, y;
} vec2_t;

/*----------------------------------------------------------------------------*/

#define VEC_ZERO ((vec3_t){ 0, 0, 0 })

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

#endif /* SDK_H_ */
