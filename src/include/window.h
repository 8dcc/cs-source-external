
#ifndef WINDOW_H_
#define WINDOW_H_ 1

#include <X11/Xlib.h>
#include <X11/extensions/Xdbe.h> /* XdbeBackBuffer */

typedef struct {
    /* Display */
    Display* disp;
    int disp_w, disp_h;
    int screen_num;

    /* Window */
    Window win;
    int win_x, win_y, win_w, win_h;
    XdbeBackBuffer backbuf;

    /* Graphic Context */
    GC gc;

    /* Fonts */
    XFontStruct* main_font;
} Context;

/*----------------------------------------------------------------------------*/

void windowInit(void);
void windowEnd(void);
void clearBackBuffer(void);
void swapBuffers(void);
void listFonts(void);

void getWindowSize(int* w, int* h);

void drawRect(int x, int y, int w, int h, uint64_t argb);
void drawFillRect(int x, int y, int w, int h, uint64_t argb);
void drawString(int x, int y, uint64_t argb, const char* str);

#endif /* WINDOW_H_ */
