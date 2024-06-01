
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/window.h"
#include "include/sdk.h"
#include "include/player.h"
#include "include/globals.h"
#include "include/util.h"

static bool WorldToScreen(const vec3_t in, vec2_t* out) {
    static float vmatrix[4][4];

    readProcessMemory(g_pid, g_viewMatrix, &vmatrix, sizeof(vmatrix));

    float w = vmatrix[3][0] * in.x + vmatrix[3][1] * in.y +
              vmatrix[3][2] * in.z + vmatrix[3][3];

    if (w < 0.01f)
        return false;

    out->x = vmatrix[0][0] * in.x + vmatrix[0][1] * in.y +
             vmatrix[0][2] * in.z + vmatrix[0][3];
    out->y = vmatrix[1][0] * in.x + vmatrix[1][1] * in.y +
             vmatrix[1][2] * in.z + vmatrix[1][3];
    float invw = 1.0f / w;

    out->x *= invw;
    out->y *= invw;

    int width, height;
    getWindowSize(&width, &height);

    float x = width / 2.0f;
    float y = height / 2.0f;

    x += 0.5f * out->x * width + 0.5f;
    y -= 0.5f * out->y * height + 0.5f;

    out->x = x;
    out->y = y;

    return true;
}

void esp(void) {
    static char name[MAX_NAME];
    static char health_str[20];

    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player* player = g_playerList + (i * 0x140);

        /* TODO: Check if the current index is the same as the localPlayer's */

        const int team           = getPlayerTeam(player);
        const uint64_t box_color = (team == 2)   ? 0xFFFF0000
                                   : (team == 3) ? 0xFF0000FF
                                                 : 0xFF222222;

        const int health = getPlayerHealth(player);
        if (health <= 0)
            continue;

        const vec3_t pos = getPlayerPos(player);
        if (vecIsZero(pos))
            continue;

        vec2_t scr;
        if (!WorldToScreen(pos, &scr))
            continue;

        getPlayerName(player, name);

        int x = scr.x - 10;
        int y = scr.y - 10;
        int w = 20;
        int h = 20;

        /* Simple box ESP */
        drawRect(x - 1, y - 1, w + 2, h + 2, 0xFF000000);
        drawRect(x, y, w, h, box_color);
        drawRect(x + 1, y + 1, w - 2, h - 2, 0xFF000000);

        /* Name and health number ESP */
        sprintf(health_str, "%d", health);
        drawString(x, y - 4, 0xFFFFFFFF, name);
        drawString(x, y - 16, 0xFFFFFFFF, health_str);

        /* Health bar */
        x -= 6;
        w               = 2;
        const int bar_h = h * MIN(health, 100) / 100;
        drawRect(x - 1, y - 1, w + 2, h + 2, 0xFF000000);
        drawFillRect(x, y, w, h, 0xFFFF0000);
        drawFillRect(x, y + h - bar_h, w, bar_h, 0xFF00FF00);
    }
}
