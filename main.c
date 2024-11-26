#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "x11_gpio.h"
#include "dlog.h"

#define GPIO_PORT_NUM 9

typedef void (*RunGuiFunc)();

renesas_gpio_t *gpio;

void *launch(void *handle)
{
    RunGuiFunc run_gui = (RunGuiFunc)dlsym(handle, "run_gui");
    ERR_RET(!run_gui, "%s", dlerror());

    dlog("launch gui");
    run_gui();
    dlog("gui fin");
error_return:
    if (handle)
        dlclose(handle);
    return NULL;
}

int main()
{
    pthread_t th;
    bool (*launched)(void);

    dlog("start");
    void *handle = dlopen("./libx11_gpio.so", RTLD_LAZY);
    ERR_RET(!handle, "%s", dlerror());

    pthread_create(&th, NULL, launch, handle);

    dlog("thread create");

    renesas_gpio_t **_gpio;
    _gpio = (renesas_gpio_t **)dlsym(handle, "renesas_gpio");
    ERR_RET(!_gpio, "%s", dlerror());

    launched = (bool (*)(void))dlsym(handle, "monitor_launched");
    ERR_RET(!launched, "%s", dlerror());

    gpio = *_gpio;
    dlog("gpio:%p, %p", gpio, _gpio);

    int port = 0;
    int pin = 0;
    dlog("first gpio:%x", gpio[0].val);

    while (!launched())
        usleep(10000);

    for (;;)
    {
continue;
        if (gpio[port].val & ((uint16_t)(1 << pin)))
        {
            gpio[port].val &= (~(uint16_t)(1 << pin));
        }
        else
        {
            gpio[port].val |= (uint16_t)(1 << pin);
        }
        dlog("%x", gpio[port].val);
        usleep(100000);
        pin++;
        if (pin >= 16)
        {
            pin = 0;
            port++;
        }

        if (port >= GPIO_PORT_NUM)
        {
            port = 0;
        }
        usleep(1);
    }

error_return:
    return 0;
}
