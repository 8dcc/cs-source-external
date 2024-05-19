
#include <stdint.h>
#include <stdio.h>
#include <unistd.h> /* usleep */

#define FPS 60

#include "include/window.h"

/*----------------------------------------------------------------------------*/

int main() {
    windowInit();

    for (;;) {
        draw();
        swapBuffers();

        usleep(1000 * 1000 / FPS);
    }

    windowEnd();
    return 0;
}
