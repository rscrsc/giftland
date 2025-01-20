#include "Sdl.h"
#include "utils.h"

#include <exception>

Sdl::Sdl (Config& cfg)
: windowTitle (cfg.title),
  windowWidth (cfg.windowWidth),
  windowHeight (cfg.windowHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error(std::format("SDL_Init: {}", SDL_GetError()));
    }
    logInfo("SDL inited with SDL_INIT_VIDEO | SDL_INIT_AUDIO");
    if (SDL_Vulkan_LoadLibrary(NULL)) {
        throw std::runtime_error(std::format("SDL_Vulkan_LoadLibrary: {}", SDL_GetError()));
    }
    logInfo("SDL loaded Vulkan");
    
    window = SDL_CreateWindow(
               windowTitle.c_str(),
               SDL_WINDOWPOS_CENTERED,
               SDL_WINDOWPOS_CENTERED,
               windowWidth, windowHeight,
               SDL_WINDOW_VULKAN |
               SDL_WINDOW_SHOWN
             );
    if (window == nullptr) {
        throw std::runtime_error(std::format("SDL_CreateWindow: {}", SDL_GetError()));
    } 
    logInfo(std::format("SDL created window < {}( {} x {} ) >", windowTitle, windowWidth, windowHeight));

    if (!SDL_Vulkan_GetInstanceExtensions(window, &vulkanInstanceExtentionCount, nullptr)) {
        throw std::runtime_error(std::format("SDL_Vulkan_GetInstanceExtension: {}", SDL_GetError()));
    }
    vulkanInstanceExtensions.resize(vulkanInstanceExtentionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &vulkanInstanceExtentionCount, vulkanInstanceExtensions.data())) {
        throw std::runtime_error(std::format("SDL_Vulkan_GetInstanceExtension: {}", SDL_GetError()));
    }

    vulkanCtx.reset(new Vulkan(vulkanInstanceExtensions, cfg));

    SDL_vulkanSurface s = nullptr;
    if (SDL_Vulkan_CreateSurface(window, vulkanCtx->getInstance(), &s) != SDL_TRUE) {
        throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
    }
    vulkanCtx->setSurface(s);
    
    vulkanCtx->createSwapchain();
    vulkanCtx->createSwapchainImageView();
    vulkanCtx->createRenderPass();
    vulkanCtx->createPipeline();
    vulkanCtx->createFramebuffer();
    vulkanCtx->createCommandBuffer();
}

Sdl::~Sdl ()
{
    SDL_DestroyWindow(window);
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();
	window = nullptr;
}

void Sdl::eventLoop ()
{
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            running = false;
        }
    }
}
