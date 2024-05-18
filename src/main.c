
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>

#include <math.h>

#define WINDOW_W 640
#define WINDOW_H 480

#define BASIC_EVENT_MASK                                                 \
    (StructureNotifyMask | ExposureMask | PropertyChangeMask |           \
     EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | \
     KeymapStateMask)
#define NOT_PROPAGATE_MASK                                                 \
    (KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
     PointerMotionMask | ButtonMotionMask)

/*----------------------------------------------------------------------------*/

typedef struct {
    Display* d;
    Window win;
    int screen_num;
    int disp_w, disp_h;
} Context;

static Context ctx;

/*----------------------------------------------------------------------------*/

static inline XColor createXColorFromRGB(uint16_t r, uint16_t g, uint16_t b) {
    XColor color;

    color.red   = r;
    color.green = g;
    color.blue  = b;
    color.flags = DoRed | DoGreen | DoBlue;

    if (!XAllocColor(ctx.d, DefaultColormap(ctx.d, ctx.screen_num), &color)) {
        fprintf(stderr, "createXColorFromRGB: Cannot create color\n");
        exit(1);
    }

    return color;
}

static inline XColor createXColorFromRGBA(uint16_t r, uint16_t g, uint16_t b,
                                          uint16_t a) {
    XColor color;

    color.red   = r;
    color.green = g;
    color.blue  = b;
    color.flags = DoRed | DoGreen | DoBlue;

    if (!XAllocColor(ctx.d, DefaultColormap(ctx.d, ctx.screen_num), &color)) {
        fprintf(stderr, "createXColorFromRGBA: Cannot create color\n");
        exit(1);
    }

    /* Add alpha in bits 24..31 */
    uint64_t pixel = (uint64_t)color.pixel;
    pixel &= 0x00FFFFFF;
    pixel |= (a & 0xFF) << 24;
    color.pixel = pixel;

    return color;
}

/*----------------------------------------------------------------------------*/

static void displayInit() {
    ctx.d = XOpenDisplay(0);
    if (!ctx.d) {
        fprintf(stderr, "Failed to open X display\n");
        exit(1);
    }

    ctx.screen_num = DefaultScreen(ctx.d);

    ctx.disp_w = DisplayWidth(ctx.d, ctx.screen_num);
    ctx.disp_h = DisplayHeight(ctx.d, ctx.screen_num);

    int shape_event_base;
    int shape_error_base;
    if (!XShapeQueryExtension(ctx.d, &shape_event_base, &shape_error_base)) {
        fprintf(stderr, "No shape extension in your system\n");
        exit(1);
    }
}

static inline void allow_input_passthrough(Window w) {
    XserverRegion region = XFixesCreateRegion(ctx.d, NULL, 0);
    XFixesSetWindowShapeRegion(ctx.d, w, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(ctx.d, region);
}

static void windowInit() {
    Window root = DefaultRootWindow(ctx.d);

    XVisualInfo vinfo;
    XMatchVisualInfo(ctx.d, DefaultScreen(ctx.d), 32, TrueColor, &vinfo);

    Colormap colormap =
      XCreateColormap(ctx.d, DefaultRootWindow(ctx.d), vinfo.visual, AllocNone);

    XSetWindowAttributes attr;
    attr.background_pixmap     = None;
    attr.background_pixel      = 0;
    attr.border_pixel          = 0;
    attr.win_gravity           = NorthWestGravity;
    attr.bit_gravity           = ForgetGravity;
    attr.save_under            = 1;
    attr.event_mask            = BASIC_EVENT_MASK;
    attr.do_not_propagate_mask = NOT_PROPAGATE_MASK;
    attr.override_redirect     = 1;    // OpenGL > 0
    attr.colormap              = colormap;

    uint64_t mask = CWColormap | CWBorderPixel | CWBackPixel | CWEventMask |
                    CWWinGravity | CWBitGravity | CWSaveUnder |
                    CWDontPropagate | CWOverrideRedirect;

    /* TODO: Change possition */
    ctx.win =
      XCreateWindow(ctx.d, root, 400, 400, WINDOW_W, WINDOW_H, 0, vinfo.depth,
                    InputOutput, vinfo.visual, mask, &attr);
    if (!ctx.win) {
        fprintf(stderr, "Failed to create X window.\n");
        exit(1);
    }

    XShapeCombineMask(ctx.d, ctx.win, ShapeInput, 0, 0, None, ShapeSet);
    XShapeSelectInput(ctx.d, ctx.win, ShapeNotifyMask);

    /* Tell the Window Manager not to draw window borders (frame) or title */
    XSetWindowAttributes wattr;
    wattr.override_redirect = 1;
    XChangeWindowAttributes(ctx.d, ctx.win, CWOverrideRedirect, &wattr);

    /* Defined above */
    allow_input_passthrough(ctx.win);

    /* Show the window */
    XMapWindow(ctx.d, ctx.win);
}

static void draw() {
    GC gc = XCreateGC(ctx.d, ctx.win, 0, 0);

    /* TODO: This could be moved out if we only used this font */
    XFontStruct* font = XLoadQueryFont(ctx.d, "fixed");
    if (!font) {
        fprintf(stderr, "Could not find \"fixed\" font\n");
        exit(1);
    }
    XSetFont(ctx.d, gc, font->fid);

    XSetBackground(ctx.d, gc, 0x00000000);

    XSetForeground(ctx.d, gc, 0xFF555555);
    XFillRectangle(ctx.d, ctx.win, gc, 30, 30, 100, 18); /* x, y, w, h */

    XSetForeground(ctx.d, gc, 0xFFFFFFFF);

    const char* str = "Hello, world.";
    XDrawString(ctx.d, ctx.win, gc, 42, 42, str, strlen(str));

    XFreeFont(ctx.d, font);
    XFreeGC(ctx.d, gc);
}

/*----------------------------------------------------------------------------*/

/* Use if needed */
void list_fonts() {
    char** fontlist;
    int num_fonts;
    fontlist = XListFonts(ctx.d, "*", 1000, &num_fonts);
    for (int i = 0; i < num_fonts; ++i)
        fprintf(stderr, "> %s\n", fontlist[i]);
}

int main() {
    displayInit();
    windowInit();

    for (;;) {
        draw();
        usleep(1000);
    }

    return 0;
}
