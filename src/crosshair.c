
#include <stdint.h>

#include "include/window.h"
#include "include/sdk.h"
#include "include/globals.h"
#include "include/player.h"

#define LEN 3
#define FOV 90

static void drawCustomCrosshair(int x, int y, uint64_t col) {
    const int top  = y - LEN;
    const int left = x - LEN;

    const int total_len = (LEN * 2) | 1;

    /* Outline */
    drawRect(x - 1, top - 1, 2, total_len + 1, 0xFF000000);
    drawRect(left - 1, y - 1, total_len + 1, 2, 0xFF000000);

    /* Crosshair */
    drawFillRect(x, top, 1, total_len, col);
    drawFillRect(left, y, total_len, 1, col);
}

void crosshair(void) {
    /* Get window size, and center of the screen */
    int w, h;
    getWindowSize(&w, &h);

    const int center_x = (w / 2) + 1;
    const int center_y = (h / 2) + 1;

    /* Normal crosshair */
    int x = center_x;
    int y = center_y;
    drawCustomCrosshair(x, y, 0xFFFFFFFF);

    /* Aim punch crosshair */
    vec3_t aimPunch = getLocalPlayerAimPunch(g_localPlayer);

    x -= (w / FOV) * aimPunch.y;
    y += (h / FOV) * aimPunch.x;

    if ((x < center_x - 1 || x > center_x + 1) &&
        (y < center_y - 1 || y > center_y + 1))
        drawCustomCrosshair(x, y, 0xFF00FF00);
}
