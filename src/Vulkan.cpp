#include "Vulkan.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <format>
#include <algorithm>
#include <limits>
#include <chrono>

void Vulkan::buildSwapchain ()
{
    createSwapchain();
    createSwapchainImageView();
    createFramebuffer();
}

void Vulkan::buildGraphicsPipeline ()
{
    createRenderPass();
    createShaderModule();
    createGraphicsPipeline();
}

void Vulkan::buildCommandBuffer ()
{
// Step 1: Create command pools (1 command pool per queue family)
    std::vector<VkCommandPoolCreateInfo> commandPoolCreateInfos;
    commandPoolCreateInfos.resize(queueFamilyInUse.size());
    commandPools.resize(queueFamilyInUse.size());
    for (uint32_t i = 0; i < commandPoolCreateInfos.size(); ++i) {
        auto& ci = commandPoolCreateInfos[i];
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.queueFamilyIndex = queueFamilyInUse[i];
        VkResult r = vkCreateCommandPool(device, &ci, nullptr, &(commandPools[i]));
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateCommandPool: {}", (int)r));
    }
// Step 2: Allocate command buffer (1 command buffer per queue family per framebuffer)
    std::vector<VkCommandBufferAllocateInfo> commandBufferAllocateInfos;
    commandBufferAllocateInfos.resize(commandPools.size());
    commandBuffers.resize(commandPools.size());
    for (uint32_t i = 0; i < commandBufferAllocateInfos.size(); ++i) {
        auto& ci = commandBufferAllocateInfos[i];
        auto& cbs = commandBuffers[i];
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ci.pNext = nullptr;
        ci.commandPool = commandPools[i];
        ci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ci.commandBufferCount = framebuffers.size();
        cbs.resize(framebuffers.size());
        VkResult r = vkAllocateCommandBuffers(device, &ci, cbs.data());
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkAllocateCommandBuffers: {}", (int)r));
    }
// Step 3: Record commands
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    {
        auto& ci = commandBufferBeginInfo;
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        ci.pNext = nullptr;
        ci.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        ci.pInheritanceInfo = nullptr;
    }
    {
    // use queue family 0 for graphics pipeline
        auto& cbs = commandBuffers[0];
        // i is index for framebuffer
        for (uint32_t i = 0; i < cbs.size(); ++i) {
        // Command buffer: Initial -> Recording
            vkBeginCommandBuffer(cbs[i], &commandBufferBeginInfo);
        // begin render pass
            std::vector<VkRenderPassBeginInfo> renderPassBeginInfos;
            std::vector<VkClearValue> attachmentClearValues;
            {
                attachmentClearValues.resize(attachments.size());
                for (auto& el : attachmentClearValues) {
                    for (size_t i = 0; i < 4; ++i) {
                        el.color.float32[i] = 0.0f;
                    }
                }
                renderPassBeginInfos.resize(framebuffers.size());
                for (uint32_t i = 0; i < framebuffers.size(); ++i) {
                    auto& ci = renderPassBeginInfos[i];
                    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    ci.pNext = nullptr;
                    ci.renderPass = renderPass;
                    ci.framebuffer = framebuffers[i];
                    ci.renderArea = VkRect2D {
                        .offset = VkOffset2D {
                            .x = 0u,
                            .y = 0u
                        },
                        .extent = surfaceCap.currentExtent
                    };
                    // clear values corresponding to attachment indices with CLEAR loadOp are used
                    ci.clearValueCount = attachmentClearValues.size();
                    ci.pClearValues = attachmentClearValues.data();
                }
            }
            vkCmdBeginRenderPass(cbs[i], &(renderPassBeginInfos[i]), VK_SUBPASS_CONTENTS_INLINE);
        // bind pipeline to command buffer of queue 0
            vkCmdBindPipeline(cbs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        // draw call
            vkCmdDraw(cbs[i], 3, 1, 0, 0);
        // end render pass
            vkCmdEndRenderPass(cbs[i]);
        // Command buffer: Recording -> Executable
            vkEndCommandBuffer(cbs[i]);
        }
    }
// Step 4: Create semaphores and fences
// - imageAvailableSemaphores : in-GPU sync, an image has been acquired and is ready for rendering
// - renderFinishedSemaphores : in-GPU sync, rendering has finished and presentation can happen
// - execFences: CPU-GPU sync, one render execution ends and the host can begin a new frame
    {
    // use queue family 0 for graphics pipeline
        auto& cbs = commandBuffers[0];
        VkSemaphoreCreateInfo semaphoreCreateInfo;
        VkFenceCreateInfo fenceCreateInfo;
        {
            auto& ci = semaphoreCreateInfo;
            ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
        }
        {
            auto& ci = fenceCreateInfo;
            ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            ci.pNext = nullptr;
            // created in signaled state
            ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }
        imageAvailableSemaphores.resize(cbs.size());
        renderFinishedSemaphores.resize(cbs.size());
        execFences.resize(cbs.size());
        for (uint32_t i = 0; i < cbs.size(); ++i) {
            VkResult r = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &(imageAvailableSemaphores[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSemaphore: {}", (int)r));
            r = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &(renderFinishedSemaphores[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSemaphore: {}", (int)r));
            r = vkCreateFence(device, &fenceCreateInfo, nullptr, &(execFences[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateFence: {}", (int)r));
        }
    }
}

void Vulkan::render ()
{
    {
        static std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> lastFrameStartTime;
        auto thisFrameStartTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        [[maybe_unused]] auto frameTime = thisFrameStartTime - lastFrameStartTime;

    // use queue family 0 queue 0 for graphics pipeline
        auto& cbs = commandBuffers[0];
        auto& q = deviceQueues[0][0];
        // we have a pool of semaphores or fences for each sync purpose, 
        // out of each pool, use one at a time (pool size no less than amount of images)
        static uint32_t syncIdx = 0;
        // correct image (canvas) to use this frame, told by swapchain later
        uint32_t imageIdx = -1;
        vkWaitForFences(device, 1, &(execFences[syncIdx]), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(device, 1, &(execFences[syncIdx]));
        // acquire (occupy) an available image to draw on
        vkAcquireNextImageKHR(device, swapchain,
            std::numeric_limits<uint64_t>::max(), 
            imageAvailableSemaphores[syncIdx],
            VK_NULL_HANDLE, &imageIdx);
        // submit command buffer to queue (once per frame)
        VkSubmitInfo submitInfo;
        // "dst" means mask out: which stages need to wait
        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        {
            auto& si = submitInfo;
            si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            si.pNext = nullptr;
            si.waitSemaphoreCount = 1;
            si.pWaitSemaphores = &(imageAvailableSemaphores[syncIdx]);
            si.pWaitDstStageMask = &waitDstStageMask;
            si.commandBufferCount = 1;
            si.pCommandBuffers = &cbs[imageIdx];
            si.signalSemaphoreCount = 1;
            si.pSignalSemaphores = &renderFinishedSemaphores[imageIdx];
        }
        // execFence is signaled when submitted command buffers have completed execution
        // command buffer state: Executable --submitted-> Pending(occupied) 
        //                                <-exec complete--      <if set one-time> --complete-> Invalid
        vkQueueSubmit(q, 1, &submitInfo, execFences[syncIdx]);
        VkPresentInfoKHR presentInfo;
        // ask queue to present to swapchain image
        {
            auto& pi = presentInfo;
            pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            pi.pNext = nullptr;
            pi.waitSemaphoreCount = 1;
            pi.pWaitSemaphores = &(renderFinishedSemaphores[syncIdx]);
            pi.swapchainCount = 1;
            pi.pSwapchains = &swapchain;
            pi.pImageIndices = &imageIdx;
            pi.pResults = nullptr;
        }
        // release the image in use and present to platform display engine
        vkQueuePresentKHR(q, &presentInfo);
        syncIdx = (syncIdx + 1) % cbs.size();
        lastFrameStartTime = thisFrameStartTime;
    }  
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
    logInfo(std::format("Queue family count: {}", queueFamilyProperties.size()));
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
        logInfo(std::format("- Queue family {}, queue count: {}", i, queueFamilyProperties[i].queueCount));
    }
    logInfo("Vulkan initialized");
}
