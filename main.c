#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "x11_gpio.h"
#include "dlog.h"

#define GPIO_PORT_NUM 9


typedef void (*RunGuiFunc)();

renesas_gpio_t *gpio;

void *launch(void *arg)
{
    void *handle = dlopen("./libx11_gpio.so", RTLD_LAZY);
    ERR_RET(!handle, "%s", dlerror());

    RunGuiFunc run_gui = (RunGuiFunc)dlsym(handle, "run_gui");
    ERR_RET(!run_gui, "%s", dlerror());

    renesas_gpio_t **_gpio = (renesas_gpio_t**)dlsym(handle, "renesas_gpio");
    ERR_RET(!_gpio, "%s", dlerror());
    gpio = *_gpio;

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
    dlog("start");
    pthread_create(&th, NULL, launch, NULL);
    dlog("thread create");

    int port = 0;
    int pin = 0;
    for (;;)
    {
        if (gpio[port].val & (uint16_t)(1 << pin))
        {
            gpio[port].val &= ~(uint16_t)(1 << pin);
        }
        else
        {
            gpio[port].val |= (uint16_t)(1 << pin);
        }
        usleep(200000);
        pin++;
        if (pin >= 16)
        {
            port++;
        }

        if (port >= GPIO_PORT_NUM)
        {
            port = 0;
        }
    }

    return 0;
}
