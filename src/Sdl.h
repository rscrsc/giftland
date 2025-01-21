#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <string>
#include <vector>
#include <memory>
#include "Vulkan.h"

class Sdl
{
private:
    SDL_Window* window;
    std::string windowTitle;
    size_t windowWidth = 0, windowHeight = 0;
    uint32_t vulkanInstanceExtentionCount;
    std::vector<const char*> vulkanInstanceExtensions;
    std::unique_ptr<Vulkan> vulkanCtx = nullptr;

    SDL_Event ev;

public:
    bool running = true;

    Sdl (Config& cfg);
    ~Sdl ();
    void eventLoop ();
    inline void render()
    {
        vulkanCtx->render();
    }
};

#endif
