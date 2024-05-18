
#ifndef WINDOW_H_
#define WINDOW_H_ 1

#include <X11/Xlib.h>

typedef struct {
    /* Display */
    Display* disp;
    int disp_w, disp_h;
    int screen_num;

    /* Window */
    Window win;
    int win_x, win_y, win_w, win_h;

    /* Graphic Context */
    GC gc;

    /* Fonts */
    XFontStruct* main_font;
} Context;

/*----------------------------------------------------------------------------*/

/* TODO: Move to different source */
void draw();

void windowInit(void);
void windowEnd(void);
void listFonts(void);

#endif /* WINDOW_H_ */