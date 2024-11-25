#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 9
#define COLS 16
#define BUTTON_SIZE 50

typedef struct
{
    Window win;
    int state; // 0: OFF, 1: ON
} Button;

Button buttons[ROWS][COLS];
Display *display;
Window root;
int screen;
GC gc;
XColor red, green;
Colormap colormap;

void toggle_button(Button *btn)
{
    btn->state = !btn->state;
    XSetForeground(display, gc, btn->state ? green.pixel : red.pixel);
    XFillRectangle(display, btn->win, gc, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
    XFlush(display);
}

void run_gui()
{
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    gc = DefaultGC(display, screen);
    colormap = DefaultColormap(display, screen);

    XAllocNamedColor(display, colormap, "red", &red, &red);
    XAllocNamedColor(display, colormap, "green", &green, &green);

    int win_width = COLS * BUTTON_SIZE;
    int win_height = ROWS * BUTTON_SIZE;
    Window window = XCreateSimpleWindow(display, root, 0, 0, win_width, win_height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
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
    while (1)
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
                        toggle_button(&buttons[i][j]);
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
            }
        }
    }

    XCloseDisplay(display);
}
