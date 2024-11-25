#ifndef __X11_GPIO_H__
#define __X11_GPIO_H__

#include <stdint.h>

typedef struct str_renesas_gpio
{

    union
    {
        uint16_t pdr;
    };
    union
    {
        uint16_t val;
        struct
        {
            uint16_t b0 : 1;
            uint16_t b1 : 1;
            uint16_t b2 : 1;
            uint16_t b3 : 1;
            uint16_t b4 : 1;
            uint16_t b5 : 1;
            uint16_t b6 : 1;
            uint16_t b7 : 1;
            uint16_t b8 : 1;
            uint16_t b9 : 1;
            uint16_t b10 : 1;
            uint16_t b11 : 1;
            uint16_t b12 : 1;
            uint16_t b13 : 1;
            uint16_t b14 : 1;
            uint16_t b15 : 1;
        };
    };
} renesas_gpio_t;

extern renesas_gpio_t *renesas_gpio;

#endif