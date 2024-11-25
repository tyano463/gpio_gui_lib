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
    Window win;
    int state; // 0: OFF, 1: ON
    int r;
    int c;
} Button;

Button buttons[ROWS][COLS];
Display *display;
Window root;
int screen;
GC gc;
XColor red, green;
Colormap colormap;

static renesas_gpio_t _gpio[GPIO_PORT_NUM];
renesas_gpio_t *renesas_gpio = _gpio;
static bool launched;

void toggle_button(Button *btn)
{
    btn->state = !btn->state;
    XSetForeground(display, gc, btn->state ? green.pixel : red.pixel);
    XFillRectangle(display, btn->win, gc, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
    XFlush(display);
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
    unsigned int xpos = dw/ 2 + 320;
    unsigned int ypos = dh/ 2 + 205;
    dlog("d:%d/%d p:%d/%d", dw, dh, xpos, ypos);

    Window window = XCreateSimpleWindow(display, root, xpos, ypos, win_width, win_height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | ButtonPressMask);
    XMapWindow(display, window);

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            int x = j * BUTTON_SIZE;
            int y = i * BUTTON_SIZE;
            buttons[i][j].win = XCreateSimpleWindow(display, window, x, y, BUTTON_SIZE, BUTTON_SIZE, 1, BlackPixel(display, screen), red.pixel);
            buttons[i][j].state = 0;
            XSelectInput(display, buttons[i][j].win, ButtonPressMask);
            XMapWindow(display, buttons[i][j].win);
        }
    }

    XEvent event;
    launched = true;
    while (1)
    {
        while (XPending(display))
        {
            XNextEvent(display, &event);
            if (event.type == ButtonPress)
            {
                for (int i = 0; i < ROWS; i++)
                {
                    for (int j = 0; j < COLS; j++)
                    {
                        if (event.xbutton.window == buttons[i][j].win)
                        {
                            renesas_gpio[i].val ^= (1 << j);
                            toggle_button(&buttons[i][j]);
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
