#include "utils.h"
#include "Vulkan.h"
#include "Sdl.h"
#include "Config.h"

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <format>

int main () {
try {
    Config cfg("config.ini");
    Sdl sdlCtx(cfg);

    while (sdlCtx.running) {
        sdlCtx.eventLoop();
        sdlCtx.render();
    }

} catch (std::runtime_error e) {
    std::cerr << "[Error] " << e.what() << std::endl;
    return 1;
}
    return 0;
}
