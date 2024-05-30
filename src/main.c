
#include <stdint.h>
#include <stdio.h>
#include <unistd.h> /* usleep */

#include "include/globals.h"
#include "include/window.h"
#include "include/features.h"

#define FPS 144

int main() {
    globalsInit();

    windowInit();

    for (;;) {
        clearBackBuffer();

        esp();
        crosshair();

        swapBuffers();
        usleep(1000 * 1000 / FPS);
    }

    windowEnd();
    return 0;
}
