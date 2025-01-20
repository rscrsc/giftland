#include "Vulkan.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <format>
#include <algorithm>
#include <limits>

void Vulkan::createInstance()
{
    prepInstanceCreateInfo();
    VkResult r = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if(r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateInstance: {}", (int)r));
}

void Vulkan::selectPhysicalDevice()
{
    // How to query a device
    // - Step 1: Get available cnt
    uint32_t physicalDevicesCnt = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCnt, nullptr);
    if (physicalDevicesCnt == 0) {
        throw std::runtime_error("no physical device found");
    }
    // - Step 2: Get available elements
    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDevicesCnt);
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCnt, physicalDevices.data());
    selectedPhysicalDevice = physicalDevices[0];
    vkGetPhysicalDeviceProperties(selectedPhysicalDevice, &physicalDeviceProperties);
}

void Vulkan::createDevice ()
{
    prepDeviceCreateInfo();
    VkResult r = vkCreateDevice(selectedPhysicalDevice, &deviceCreateInfo, nullptr, &device);
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateDevice: {}", (int)r));
}

void Vulkan::getDeviceQueues () {
    deviceQueues.resize(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        uint32_t queueCnt = queueFamilyProperties[i].queueCount;
        for (uint32_t j = 0; j < queueCnt; ++j) {
            VkQueue q;
            vkGetDeviceQueue(device, i, j, &q);
            deviceQueues[i].push_back(q);
        }
    }
}

void Vulkan::createSwapchainImageView ()
{
    uint32_t swapchainImageCnt;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCnt, nullptr);
    swapchainImages.resize(swapchainImageCnt);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCnt, swapchainImages.data());
    prepSwapchainImageViewCreateInfo();
    swapchainImageViews.resize(swapchainImageViewCreateInfos.size());
    for (uint32_t i = 0; i < swapchainImageViewCreateInfos.size(); ++i) {
        VkResult r = vkCreateImageView(device, &(swapchainImageViewCreateInfos[i]), nullptr, &(swapchainImageViews[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateImageView: {}", (int)r));
    }
}

void Vulkan::createRenderPass ()
{
    prepRenderPassCreateInfo();
    VkResult r = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateRenderPass: {}", (int)r));
}

void Vulkan::selectFormat ()
{
    uint32_t surfaceFormatCnt = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, surface, &surfaceFormatCnt, nullptr);
    if (surfaceFormatCnt == 0) {
        throw std::runtime_error("no supported pixel format found");
    }
    std::vector<VkSurfaceFormatKHR> supportedFormats;
    supportedFormats.resize(surfaceFormatCnt);
    VkResult r = vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, surface, &surfaceFormatCnt, supportedFormats.data());
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkGetPhysicalDeviceSurfaceFormatsKHR: {}", (int)r));
    bool isFormatSupported = false;
    for (auto& el : supportedFormats) {
        if (el.format == VK_FORMAT_B8G8R8A8_SRGB && el.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selectedSurfaceFormat = el;
            isFormatSupported = true;
        }
    }
    if (!isFormatSupported) {
        throw std::runtime_error("no compatible pixel format supported");
    }
}

void Vulkan::createShaderModule ()
{
    prepShaderModuleCreateInfo();
    VkResult r = vkCreateShaderModule(device, &(shaderModuleCreateInfos.vert), nullptr, &(shaderModules.vert));
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateShaderModule: {}", (int)r));
    r = vkCreateShaderModule(device, &(shaderModuleCreateInfos.frag), nullptr, &(shaderModules.frag));
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateShaderModule: {}", (int)r));
}

void Vulkan::createPipeline ()
{
    createShaderModule();
    prepShaderStageCreateInfo();
    prepVertexInputStateCreateInfo();
    prepInputAssemblyStateCreateInfo();
    // prepTessellationStateCreateInfo();
    prepViewportStateCreateInfo();
    prepRasterizationStateCreateInfo();
    prepMultisampleStateCreateInfo();
    // prepDepthStencilStateCreateInfo();
    prepColorBlendStateCreateInfo();
    prepDynamicStateCreateInfo();
    prepGraphicsPipelineCreateInfo();
    VkResult r = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateGraphicsPipelines: {}", (int)r));
}

void Vulkan::createFramebuffer ()
{
    prepFramebufferCreateInfo();
    framebuffers.resize(framebufferCreateInfos.size());
    for (uint32_t i = 0; i < framebufferCreateInfos.size(); ++i) {
        VkResult r = vkCreateFramebuffer(device, &(framebufferCreateInfos[i]), nullptr, &(framebuffers[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateFramebuffer: {}", (int)r));
    }
}

void Vulkan::createCommandBuffer ()
{
// Step1 create command pools
    prepCommandPoolCreateInfo();
    commandPools.resize(commandPoolCreateInfos.size());
    for (uint32_t i = 0; i < commandPoolCreateInfos.size(); ++i) {
        VkResult r = vkCreateCommandPool(device, &(commandPoolCreateInfos[i]), nullptr, &(commandPools[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateCommandPool: {}", (int)r));
    }
// Step2 allocate command buffer
    prepCommandBufferAllocateInfo();
    commandBuffers.resize(queueFamilyInUse.size());
    for (uint32_t i = 0; i < queueFamilyInUse.size(); ++i) {
        auto& cb = commandBuffers[i];
        cb.resize(framebuffers.size());
        VkResult r = vkAllocateCommandBuffers(device, &(commandBufferAllocateInfos[i]), cb.data());
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkAllocateCommandBuffers: {}", (int)r));
    }
// Step3 record init commands
    for (uint32_t i = 0; i < commandBuffers.size(); ++i) {
        prepCommandBufferBeginInfo();
        for (uint32_t j = 0; j < commandBuffers[i].size(); ++j) {
            auto& cb = commandBuffers[i][j];
        // Initial -> Recording
            vkBeginCommandBuffer(cb, &commandBufferBeginInfo);
            prepRenderPassBeginInfo();
            vkCmdBeginRenderPass(cb, &(renderPassBeginInfos[j]), VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdDraw(cb, 3, 1, 0, 0);
            vkCmdEndRenderPass(cb);
        // Recording -> Executable
            vkEndCommandBuffer(cb);
        }
    }
// Step4 create semaphores and fences
// - semaphore1 : an image has been acquired and is ready for rendering
// - semaphore2 : rendering has finished and presentation can happen
    imageAvailableSemaphores.resize(commandBuffers.size());
    renderFinishedSemaphores.resize(commandBuffers.size());
    inFlightFences.resize(commandBuffers.size());
    prepSemaphoreCreateInfo();
    prepFenceCreateInfo();
    for (uint32_t i = 0; i < commandBuffers[0].size(); ++i) {
        VkResult r = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &(imageAvailableSemaphores[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSemaphore: {}", (int)r));
        r = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &(renderFinishedSemaphores[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSemaphore: {}", (int)r));
        r = vkCreateFence(device, &fenceCreateInfo, nullptr, &(inFlightFences[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateFence: {}", (int)r));
    }
}

void Vulkan::createSwapchain ()
{
    prepSwapchainCreateInfo();
    VkResult r = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSwapchainKHR: ", (int)r));
}

void Vulkan::render ()
{
    static uint32_t currentFrame = 0;
	static uint32_t imageIndex = -1;
    vkWaitForFences(device, 1, &(inFlightFences[currentFrame]), VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &(inFlightFences[currentFrame]));
    vkAcquireNextImageKHR(device, swapchain, 
        std::numeric_limits<uint64_t>::max(), 
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);
    // TODO : multi queue parallel
    prepSubmitInfo(0, currentFrame, imageIndex);
    vkQueueSubmit(deviceQueues[0][0], 1, &submitInfo, 
        inFlightFences[currentFrame]);
    prepPresentInfo(currentFrame, imageIndex);
    vkQueuePresentKHR(deviceQueues[0][0], &presentInfo);
    //std::cout << "image index " << imageIndex << "\n";
	//std::cout << "current frame " << currentFrame << "\n";

	currentFrame = (currentFrame + 1) % commandBuffers[0].size();    
}

Vulkan::~Vulkan ()
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // After destroying all objs created with device, wait idle and destroy it
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}

Vulkan::Vulkan (std::vector<const char*> additionalInctanceExtensions, Config& cfg)
: cfg (cfg)
{
    logInfo("Initializing Vulkan ...");

    instanceEnabledExtensionNames.insert(instanceEnabledExtensionNames.end(), additionalInctanceExtensions.begin(), additionalInctanceExtensions.end());
    createInstance();
    selectPhysicalDevice();
    logInfo(std::format("Selected phy device: {}", physicalDeviceProperties.deviceName));

    createDevice();
    getDeviceQueues();
    logInfo(std::format("Queue family count: {}", queueFamilyCount));
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        logInfo(std::format("- Queue family {}, queue count: {}", i, queueFamilyProperties[i].queueCount));
    }
    logInfo("Vulkan initialized");
}
