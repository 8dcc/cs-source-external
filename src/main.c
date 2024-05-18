
#include <stdint.h>
#include <stdio.h>
#include <unistd.h> /* usleep */

#include "include/window.h"

/*----------------------------------------------------------------------------*/

int main() {
    windowInit();

    for (;;) {
        draw();
        usleep(1000);
    }

    windowEnd();
    return 0;
}
