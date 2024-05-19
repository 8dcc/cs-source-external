#include <stdint.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// TODO: Filter includes
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdbe.h> /* XdbeBackBuffer */

#include <math.h>

#include "include/window.h"

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

#define WINDOW_X 400
#define WINDOW_Y 400
#define WINDOW_W 640
#define WINDOW_H 480

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
        fprintf(stderr, "createXColorFromRGB: Cannot create color\n");
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
        fprintf(stderr, "createXColorFromRGBA: Cannot create color\n");
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
        fprintf(stderr, "Failed to open X display\n");
        exit(1);
    }

    ctx.screen_num = DefaultScreen(ctx.disp);
    ctx.disp_w     = DisplayWidth(ctx.disp, ctx.screen_num);
    ctx.disp_h     = DisplayHeight(ctx.disp, ctx.screen_num);

    int shape_event_base, shape_error_base;
    if (!XShapeQueryExtension(ctx.disp, &shape_event_base, &shape_error_base)) {
        fprintf(stderr, "No shape extension in your system\n");
        exit(1);
    }
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

    /* TODO: Change position and size depending on the game's */
    ctx.win_x = WINDOW_X;
    ctx.win_y = WINDOW_Y;
    ctx.win_w = WINDOW_W;
    ctx.win_h = WINDOW_H;

    ctx.win = XCreateWindow(ctx.disp, root, ctx.win_x, ctx.win_y, ctx.win_w,
                            ctx.win_h, 0, vinfo.depth, InputOutput,
                            vinfo.visual, MY_WINDOW_MASK, &attr);
    if (!ctx.win) {
        fprintf(stderr, "Failed to create X window.\n");
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
        fprintf(stderr, "XDBE is not supported.\n");
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
        fprintf(stderr, "Could not find font \"%s\"\n", FONT_NAME);
        exit(1);
    }

    /* We will only use a single font */
    XSetFont(ctx.disp, ctx.gc, ctx.main_font->fid);
}

/*----------------------------------------------------------------------------*/

void windowInit(void) {
    initDisplay();
    initWindow();
    initGraphicContext();
    initFont();
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

/* TODO: Move to different source */
void draw(void) {
    /* Clear the back buffer */
    XSetForeground(ctx.disp, ctx.gc, 0x00000000);
    XFillRectangle(ctx.disp, ctx.backbuf, ctx.gc, 0, 0, ctx.win_w, ctx.win_h);

    /* Rect: x, y, w, h */
    XSetForeground(ctx.disp, ctx.gc, 0xFF555555);
    XFillRectangle(ctx.disp, ctx.backbuf, ctx.gc, 30, 30, 100, 18);

    XSetForeground(ctx.disp, ctx.gc, 0xFFFFFFFF);

    const char* str = "Hello, world.";
    XDrawString(ctx.disp, ctx.backbuf, ctx.gc, 42, 42, str, strlen(str));
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
        fprintf(stderr, "> %s\n", fontlist[i]);
}
