
#include <stdint.h>
#include <stdio.h>
#include <unistd.h> /* usleep */

#define FPS 60

#include "include/window.h"

/*----------------------------------------------------------------------------*/

int main() {
    windowInit();

    for (;;) {
        clearBackBuffer();

        drawRect(30, 30, 100, 18, 0xFF555555);
        drawString(42, 42, 0xFFFFFFFF, "Hello, world!");

        swapBuffers();
        usleep(1000 * 1000 / FPS);
    }

    windowEnd();
    return 0;
}
