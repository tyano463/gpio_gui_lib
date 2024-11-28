#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "x11_gpio.h"
#include "dlog.h"

#define GPIO_PORT_NUM 9

#define ROWS GPIO_PORT_NUM
#define COLS 16
#define BUTTON_SIZE 50

typedef struct
{
    Display *display;
    int screen;
    GC gc;
    Window win;
    int state; // 0: OFF, 1: ON
    int r;
    int c;
    char label[4];
} Button;

static Button buttons[ROWS][COLS];
static Display *display;
static Window root;
static int screen;
static GC gc;
static XColor red, green;
static Colormap colormap;

static renesas_gpio_t _gpio[GPIO_PORT_NUM];
renesas_gpio_t *renesas_gpio = _gpio;
static bool launched;

static XFontSet create_fontset(Display *display)
{
    XFontSet fontset;
    char **missing_charset_list;
    int missing_charset_count = 0;
    char *default_string = NULL;

    fontset = XCreateFontSet(display, "-*-*-medium-r-normal--16-*-*-*-*-*-*-*",
                             &missing_charset_list, &missing_charset_count,
                             &default_string);

    if (missing_charset_count > 0)
    {
        int exist = 0;
        for (int i = 0; i < missing_charset_count; i++)
        {
            if (strcmp("ISO8859-13", missing_charset_list[i]) == 0)
                continue; // ポーランドは無視
            if (strcmp("ISO8859-14", missing_charset_list[i]) == 0)
                continue; // ケルト語も無視
            if (strcmp("KSC5601.1987-0", missing_charset_list[i]) == 0)
                continue; // 韓国語も無視

            if (!exist)
            {
                exist = 1;
                dlog("Missing charsets:\n");
            }
            dlog("  %s\n", missing_charset_list[i]);
        }
        XFreeStringList(missing_charset_list);
    }

    if (default_string != NULL && default_string[0])
    {
        dlog("Default string: %s\n", default_string);
    }
    return fontset;
}

static void rgb2xcolor(XColor *color, uint8_t r, uint8_t g, uint8_t b)
{
    Status status;
    color->red = r * 256;
    color->green = g * 256;
    color->blue = b * 256;

    color->flags = DoRed | DoGreen | DoBlue;
    status = XAllocColor(display, colormap, color);
}

static void draw_button_label(Button *btn)
{
    Status status;
    XColor color;
    XFontSet fontset;

    fontset = create_fontset(display);
    rgb2xcolor(&color, 0, 0, 0);

    XSetForeground(btn->display, btn->gc, color.pixel);
    Xutf8DrawString(btn->display, btn->win, fontset, btn->gc, 15, 30, btn->label, strlen(btn->label));
}

void toggle_button(Button *btn)
{
    btn->state = !btn->state;
    XSetForeground(display, gc, btn->state ? green.pixel : red.pixel);
    XFillRectangle(display, btn->win, gc, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
    XFlush(display);

    draw_button_label(btn);
}

bool monitor_launched(void)
{
    return launched;
}

void run_gui()
{
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    dlog("gpio:%p", renesas_gpio);

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    gc = DefaultGC(display, screen);
    colormap = DefaultColormap(display, screen);

    XAllocNamedColor(display, colormap, "white", &red, &red);
    XAllocNamedColor(display, colormap, "green", &green, &green);

    int win_width = COLS * BUTTON_SIZE;
    int win_height = ROWS * BUTTON_SIZE;
    unsigned int dw = DisplayWidth(display, screen);
    unsigned int dh = DisplayHeight(display, screen);
    unsigned int xpos = dw / 2 + 320;
    unsigned int ypos = dh / 2 + 205;
    dlog("d:%d/%d p:%d/%d", dw, dh, xpos, ypos);

    Window window = XCreateSimpleWindow(display, root, xpos, ypos, win_width, win_height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | ButtonPressMask);
    XMapWindow(display, window);

    char label[4];
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            int x = j * BUTTON_SIZE;
            int y = i * BUTTON_SIZE;
            Button *btn = &buttons[i][j];
            btn->win = XCreateSimpleWindow(display, window, x, y, BUTTON_SIZE, BUTTON_SIZE, 1, BlackPixel(display, screen), red.pixel);
            btn->state = 0;
            XSelectInput(display, btn->win, ButtonPressMask | ButtonReleaseMask);
            XMapWindow(display, btn->win);

            btn->display = display;
            btn->screen = screen;
            btn->gc = gc;

            snprintf(btn->label, 4, "%d%02d", i, j);
            draw_button_label(btn);
        }
    }

    XEvent event;
    launched = true;
    while (1)
    {
        while (XPending(display))
        {
            XNextEvent(display, &event);
            if (event.type == ButtonPress || event.type == ButtonRelease)
            {
                for (int i = 0; i < ROWS; i++)
                {
                    for (int j = 0; j < COLS; j++)
                    {
                        if (event.xbutton.window == buttons[i][j].win)
                        {
                            renesas_gpio[i].val ^= (1 << j);
                            toggle_button(&buttons[i][j]);
                            dlog("%d%02d:%08x", i, j, renesas_gpio[i].val);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                int gpio_state = (renesas_gpio[i].val & (1 << j)) ? 1 : 0;
                if (gpio_state != buttons[i][j].state)
                {
                    toggle_button(&buttons[i][j]);
                }
            }
        }

        usleep(10000);
    }

    dlog("loop exit");

    XCloseDisplay(display);
error_return:
    return;
}
