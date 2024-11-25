#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef void (*RunGuiFunc)();

int main() {
    void *handle = dlopen("./libx11_gpio.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }

    RunGuiFunc run_gui = (RunGuiFunc)dlsym(handle, "run_gui");
    if (!run_gui) {
        fprintf(stderr, "%s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    run_gui();

    dlclose(handle);
    return 0;
}
