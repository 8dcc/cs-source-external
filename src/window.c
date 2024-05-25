
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>  /* XShapeCombineMask */
#include <X11/extensions/Xfixes.h> /* XFixesCreateRegion */
#include <X11/extensions/Xdbe.h>   /* XdbeBackBuffer */

#include <math.h>

#include "include/window.h"
#include "include/util.h" /* ERR macro */

/*----------------------------------------------------------------------------*/

/* TODO: Explain masks */
#define BASIC_EVENT_MASK                                                 \
    (StructureNotifyMask | ExposureMask | PropertyChangeMask |           \
     EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | \
     KeymapStateMask)

#define NOT_PROPAGATE_MASK                                                 \
    (KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
     PointerMotionMask | ButtonMotionMask)

#define MY_WINDOW_MASK                                                       \
    (CWColormap | CWBorderPixel | CWBackPixel | CWEventMask | CWWinGravity | \
     CWBitGravity | CWSaveUnder | CWDontPropagate | CWOverrideRedirect)

#define FONT_NAME "fixed"

/*----------------------------------------------------------------------------*/

/* Create our own context with global variables.
 * TODO: Make local in main? */
static Context ctx;

/*----------------------------------------------------------------------------*/

static inline XColor rgb2xcolor(uint16_t r, uint16_t g, uint16_t b) {
    XColor color;

    color.red   = r;
    color.green = g;
    color.blue  = b;
    color.flags = DoRed | DoGreen | DoBlue;

    if (!XAllocColor(ctx.disp, DefaultColormap(ctx.disp, ctx.screen_num),
                     &color)) {
        ERR("Cannot create color");
        exit(1);
    }

    return color;
}

static inline XColor rgba2xcolor(uint16_t r, uint16_t g, uint16_t b,
                                 uint16_t a) {
    XColor color;

    color.red   = r;
    color.green = g;
    color.blue  = b;
    color.flags = DoRed | DoGreen | DoBlue;

    if (!XAllocColor(ctx.disp, DefaultColormap(ctx.disp, ctx.screen_num),
                     &color)) {
        ERR("Cannot create color");
        exit(1);
    }

    /* The pixel format is ARGB. Add alpha in bits 24..31 */
    uint64_t pixel = (uint64_t)color.pixel;
    pixel &= 0x00FFFFFF;
    pixel |= (a & 0xFF) << 24;
    color.pixel = pixel;

    return color;
}

/*----------------------------------------------------------------------------*/

static void initDisplay(void) {
    ctx.disp = XOpenDisplay(NULL);
    if (!ctx.disp) {
        ERR("Failed to open X display");
        exit(1);
    }

    ctx.screen_num = DefaultScreen(ctx.disp);
    ctx.disp_w     = DisplayWidth(ctx.disp, ctx.screen_num);
    ctx.disp_h     = DisplayHeight(ctx.disp, ctx.screen_num);

    int shape_event_base, shape_error_base;
    if (!XShapeQueryExtension(ctx.disp, &shape_event_base, &shape_error_base)) {
        ERR("No shape extension in your system");
        exit(1);
    }
}

#define INVALID_WINDOW 0
static Window getWindowFromName(Window window, const char* name) {
    char* cur_name;
    XFetchName(ctx.disp, window, &cur_name);

    if (cur_name != NULL && !strcmp(cur_name, name))
        return window;

    Window root, parent;
    Window* children;
    unsigned int num;
    XQueryTree(ctx.disp, window, &root, &parent, &children, &num);

    if (children == NULL)
        return INVALID_WINDOW;

    for (unsigned int i = 0; i < num; i++) {
        Window ret = getWindowFromName(children[i], name);
        if (ret != INVALID_WINDOW) {
            XFree(children);
            return ret;
        }
    }

    XFree(children);
    return INVALID_WINDOW;
}

#define CS_WINNAME "Counter-Strike Source - OpenGL"
static void initWindowPosition(void) {
    Window root = DefaultRootWindow(ctx.disp);

    Window game = getWindowFromName(root, CS_WINNAME);
    if (game == INVALID_WINDOW) {
        ERR("Could not find a window with name: \"%s\".", CS_WINNAME);
        exit(1);
    }

    XWindowAttributes win_attr;
    XGetWindowAttributes(ctx.disp, game, &win_attr);

    /* Change position and size depending on the game's */
    ctx.win_x = win_attr.x;
    ctx.win_y = win_attr.y;
    ctx.win_w = win_attr.width;
    ctx.win_h = win_attr.height;
}

static void initWindow(void) {
    Window root = DefaultRootWindow(ctx.disp);

    /* Get display info, will be needed when creating the window bellow */
    XVisualInfo vinfo;
    XMatchVisualInfo(ctx.disp, ctx.screen_num, 32, TrueColor, &vinfo);

    /* Create colormap used for the window attributes */
    Colormap colormap =
      XCreateColormap(ctx.disp, root, vinfo.visual, AllocNone);

    /* Set the attributes for the window */
    XSetWindowAttributes attr;
    attr.background_pixmap     = None;
    attr.background_pixel      = 0;
    attr.border_pixel          = 0;
    attr.win_gravity           = NorthWestGravity;
    attr.bit_gravity           = ForgetGravity;
    attr.save_under            = 1;
    attr.event_mask            = BASIC_EVENT_MASK;
    attr.do_not_propagate_mask = NOT_PROPAGATE_MASK;
    attr.override_redirect     = 1;
    attr.colormap              = colormap;

    for (;;) {
        initWindowPosition();
        if (ctx.win_x >= 0 && ctx.win_y >= 0)
            break;

        fprintf(stderr,
                "Could not get a valid position for \"%s\".\n"
                "Press RET to retry...",
                CS_WINNAME);
        getchar();
    }

    ctx.win = XCreateWindow(ctx.disp, root, ctx.win_x, ctx.win_y, ctx.win_w,
                            ctx.win_h, 0, vinfo.depth, InputOutput,
                            vinfo.visual, MY_WINDOW_MASK, &attr);
    if (!ctx.win) {
        ERR("Failed to create X window.");
        exit(1);
    }

    /* TODO: Explanation */
    XShapeCombineMask(ctx.disp, ctx.win, ShapeInput, 0, 0, None, ShapeSet);
    XShapeSelectInput(ctx.disp, ctx.win, ShapeNotifyMask);

    /* Tell the Window Manager not to draw window borders (frame) or title */
    XSetWindowAttributes wattr;
    wattr.override_redirect = 1;
    XChangeWindowAttributes(ctx.disp, ctx.win, CWOverrideRedirect, &wattr);

    /* Allow input passthrough */
    XserverRegion region = XFixesCreateRegion(ctx.disp, NULL, 0);
    XFixesSetWindowShapeRegion(ctx.disp, ctx.win, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(ctx.disp, region);

    int major, minor;
    if (!XdbeQueryExtension(ctx.disp, &major, &minor)) {
        ERR("XDBE is not supported.");
        exit(1);
    }

    /* Allocate the back buffer */
    ctx.backbuf = XdbeAllocateBackBufferName(ctx.disp, ctx.win, 0);

    /* Show the window */
    XMapWindow(ctx.disp, ctx.win);
}

static void initGraphicContext(void) {
    /* Create graphic context */
    ctx.gc = XCreateGC(ctx.disp, ctx.win, 0, 0);

    /* No alpha */
    XSetBackground(ctx.disp, ctx.gc, 0x00000000);

    /* Clear the window initially */
    XClearWindow(ctx.disp, ctx.win);

    /* Move window to the top of the queue */
    XMapRaised(ctx.disp, ctx.win);
}

static void initFont(void) {
    /* Load the main font */
    ctx.main_font = XLoadQueryFont(ctx.disp, FONT_NAME);
    if (!ctx.main_font) {
        ERR("Could not find font \"%s\"", FONT_NAME);
        exit(1);
    }

    /* We will only use a single font */
    XSetFont(ctx.disp, ctx.gc, ctx.main_font->fid);
}

/*----------------------------------------------------------------------------*/

void windowInit(void) {
    initDisplay();
    printf("Display initialized. Default screen: %d, %dx%d.\n", ctx.screen_num,
           ctx.disp_w, ctx.disp_h);

    initWindow();
    printf("Window initialized. Pos: (%d,%d) Dimensions: %dx%d.\n", ctx.win_x,
           ctx.win_y, ctx.win_w, ctx.win_h);

    initGraphicContext();
    printf("Graphic Context initialized.\n");

    initFont();
    printf("Font \"%s\" initialized.\n", FONT_NAME);
}

void windowEnd(void) {
    /* From initFont */
    XFreeFont(ctx.disp, ctx.main_font);

    /* From initGraphicContext */
    XFreeGC(ctx.disp, ctx.gc);

    /* From initWindow() */
    XDestroyWindow(ctx.disp, ctx.win);

    /* From initDisplay() */
    XCloseDisplay(ctx.disp);
}

void clearBackBuffer(void) {
    XSetForeground(ctx.disp, ctx.gc, 0x00000000);
    XFillRectangle(ctx.disp, ctx.backbuf, ctx.gc, 0, 0, ctx.win_w, ctx.win_h);
}

void swapBuffers(void) {
    /* Specify the window and action, and swap the front and back buffers */
    XdbeSwapInfo swap_info = {
        .swap_window = ctx.win,
        .swap_action = XdbeUndefined,
    };
    XdbeSwapBuffers(ctx.disp, &swap_info, 1);

    /* We need to perform an explicit XFlush() */
    XFlush(ctx.disp);
}

/* Use when needed */
void listFonts(void) {
    int num_fonts;
    char** fontlist = XListFonts(ctx.disp, "*", 1000, &num_fonts);
    for (int i = 0; i < num_fonts; ++i)
        printf("> %s\n", fontlist[i]);
}

/*----------------------------------------------------------------------------*/

void getWindowSize(int* w, int* h) {
    *w = ctx.win_w;
    *h = ctx.win_h;
}

/*----------------------------------------------------------------------------*/

void drawRect(int x, int y, int w, int h, uint64_t argb) {
    XSetForeground(ctx.disp, ctx.gc, argb & 0xFFFFFFFF);
    XDrawRectangle(ctx.disp, ctx.backbuf, ctx.gc, x, y, w, h);
}

void drawFillRect(int x, int y, int w, int h, uint64_t argb) {
    XSetForeground(ctx.disp, ctx.gc, argb & 0xFFFFFFFF);
    XFillRectangle(ctx.disp, ctx.backbuf, ctx.gc, x, y, w, h);
}

void drawString(int x, int y, uint64_t argb, const char* str) {
    XSetForeground(ctx.disp, ctx.gc, argb & 0xFFFFFFFF);
    XDrawString(ctx.disp, ctx.backbuf, ctx.gc, x, y, str, strlen(str));
}
