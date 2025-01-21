#ifndef VULKAN_H
#define VULKAN_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <cassert>
#include <algorithm>
#include <memory>
#include <limits>
#include "utils.h"
#include "Config.h"

class Vulkan
{
private:
    [[maybe_unused]] Config& cfg;
    VkInstance instance;
    VkInstanceCreateInfo instanceCreateInfo;
    VkApplicationInfo applicationInfo;
    std::vector<const char*> instanceEnabledExtensionNames = {"VK_KHR_portability_enumeration"};

    VkPhysicalDevice selectedPhysicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<std::vector<float>> queueFamilyPriorities;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    VkDeviceCreateInfo deviceCreateInfo;
    std::vector<const char*> deviceEnabledExtensionNames = {"VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkDevice device;
    std::vector<std::vector<VkQueue>> deviceQueues;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    VkSurfaceFormatKHR selectedSurfaceFormat;
    VkSurfaceCapabilitiesKHR surfaceCap;
    std::vector<uint32_t> queueFamilyInUse = {0};

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageViewCreateInfo> swapchainImageViewCreateInfos;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkClearValue> attachmentClearValues;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
    VkRenderPassCreateInfo renderPassCreateInfo;
    VkRenderPass renderPass;

    std::vector<std::vector<char>> shaderCodes;
    std::vector<VkShaderModuleCreateInfo> shaderModuleCreateInfos;
    std::vector<VkShaderModule> shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    struct {
        VkPipelineVertexInputStateCreateInfo vertexInput;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineTessellationStateCreateInfo tessellation;
        VkPipelineViewportStateCreateInfo viewport;
        VkPipelineRasterizationStateCreateInfo rasterization;
        VkPipelineMultisampleStateCreateInfo multisample;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineColorBlendStateCreateInfo colorBlend;
        VkPipelineDynamicStateCreateInfo dynamic;
    } pipelineStateCreateInfos;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    std::vector<VkDescriptorSetLayout> descriptorSetLayout;
    std::vector<VkPushConstantRange> pushConstantRanges;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    VkPipelineLayout pipelineLayout;
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    VkPipeline pipeline;

    std::vector<VkFramebufferCreateInfo> framebufferCreateInfos;
    std::vector<VkFramebuffer> framebuffers;

    std::vector<VkCommandPoolCreateInfo> commandPoolCreateInfos;
    std::vector<VkCommandPool> commandPools;
    std::vector<VkCommandBufferAllocateInfo> commandBufferAllocateInfos;
// dim: queueFamilyInUse.size() * framebuffers.size()
    std::vector<std::vector<VkCommandBuffer>> commandBuffers;
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    std::vector<VkRenderPassBeginInfo> renderPassBeginInfos;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

//    VkBufferCreateInfo vertexBufferCreateInfo;

    inline void createInstance()
    {
        {
            auto& ai = applicationInfo;
            ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            ai.pNext = nullptr;
            ai.pApplicationName = "Giftland";
            ai.applicationVersion = 1;
            ai.pEngineName = "Rxon";
            ai.engineVersion = 1;
            ai.apiVersion = VK_API_VERSION_1_0;
        }
        auto& ci = instanceCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        ci.pApplicationInfo = &applicationInfo;
        ci.enabledLayerCount = 0;
        ci.ppEnabledLayerNames = {};
        ci.enabledExtensionCount = instanceEnabledExtensionNames.size();
        ci.ppEnabledExtensionNames = instanceEnabledExtensionNames.data();
        VkResult r = vkCreateInstance(&ci, nullptr, &instance);
        if(r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateInstance: {}", (int)r));
    }
    inline void selectPhysicalDevice()
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
    inline void createDevice ()
    {
    // Step 1: Get queue family info
        {
            uint32_t queueFamilyCnt = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCnt, nullptr);
            queueFamilyProperties.resize(queueFamilyCnt);
            vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCnt, queueFamilyProperties.data());
        }
    // Step 2: Prepare queue create info
        queueCreateInfos.resize(queueFamilyProperties.size());
        queueFamilyPriorities.resize(queueFamilyProperties.size());
        for (uint32_t i = 0; i < queueCreateInfos.size(); ++i) {
            auto& ci = queueCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.queueFamilyIndex = i;
            ci.queueCount = queueFamilyProperties[i].queueCount;
            queueFamilyPriorities[i] = std::vector<float>(queueFamilyProperties[i].queueCount, 0.5);
            ci.pQueuePriorities = queueFamilyPriorities[i].data();
        }
    // Step 3: Create device (implicitly created queues)
        auto& ci = deviceCreateInfo;
        vkGetPhysicalDeviceFeatures(selectedPhysicalDevice, &physicalDeviceFeatures);
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.queueCreateInfoCount = queueCreateInfos.size();
        ci.pQueueCreateInfos = queueCreateInfos.data();
        ci.enabledLayerCount = 0;
        ci.ppEnabledLayerNames = nullptr;
        ci.enabledExtensionCount = deviceEnabledExtensionNames.size();
        ci.ppEnabledExtensionNames = deviceEnabledExtensionNames.data();
        ci.pEnabledFeatures = &physicalDeviceFeatures;
        VkResult r = vkCreateDevice(selectedPhysicalDevice, &ci, nullptr, &device);
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateDevice: {}", (int)r));
    }
    inline void getDeviceQueues () {
        deviceQueues.resize(queueFamilyProperties.size());
        for (uint32_t i = 0; i < deviceQueues.size(); ++i) {
            auto& sameFamilyQueues = deviceQueues[i];
            sameFamilyQueues.resize(queueFamilyProperties[i].queueCount);
            for (uint32_t j = 0; j < sameFamilyQueues.size(); ++j) {
                vkGetDeviceQueue(device, i, j, sameFamilyQueues.data());
            }
        }
    }
    inline void selectFormat ()
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
    inline void createSwapchain ()
    {
        selectFormat();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedPhysicalDevice, surface, &surfaceCap);

        auto& ci = swapchainCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.surface = surface;
        ci.minImageCount = std::max(surfaceCap.minImageCount, 2u);
        ci.imageFormat = selectedSurfaceFormat.format;
        ci.imageColorSpace = selectedSurfaceFormat.colorSpace;
        ci.imageExtent = surfaceCap.currentExtent;
    // non-stereoscopic-3D applications
        ci.imageArrayLayers = 1;
    // what you may use the swapchain as
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // which queue family first use the swapchain get the ownership
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.queueFamilyIndexCount = queueFamilyInUse.size();
        ci.pQueueFamilyIndices = queueFamilyInUse.data();
    // transforms like rotate, flip, etc
        ci.preTransform = surfaceCap.currentTransform;
    // to be controlled by windowing system, use VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // "present" means the process that the image passed after renderred
        ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    // are pixels removed because not seen or some other reasons
        ci.clipped = VK_TRUE;
        ci.oldSwapchain = VK_NULL_HANDLE;
        VkResult r = vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain);
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateSwapchainKHR: ", (int)r));
    }
/*
    inline void prepVertexBufferCreateInfo (size_t bufferSize)
    {
        auto& vbci = vertexBufferCreateInfo;
        vbci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vbci.pNext = nullptr;
        vbci.flags = 0;
        vbci.size = bufferSize;
        vbci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vbci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vbci.queueFamilyIndexCount = queueFamilyInUse.size();
        vbci.pQueueFamilyIndices = queueFamilyInUse.data();
    }
*/
    inline void createSwapchainImageView ()
    {
        {
            uint32_t swapchainImageCnt = 0;
            vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCnt, nullptr);
            swapchainImages.resize(swapchainImageCnt);
            vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCnt, swapchainImages.data());
        }
        swapchainImageViewCreateInfos.resize(swapchainImages.size());
        swapchainImageViews.resize(swapchainImages.size());
        for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
            auto& ci = swapchainImageViewCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.image = swapchainImages[i];
            ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ci.format = selectedSurfaceFormat.format;
            ci.components = VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            };
            ci.subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            VkResult r = vkCreateImageView(device, &(swapchainImageViewCreateInfos[i]), nullptr, &(swapchainImageViews[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateImageView: {}", (int)r));
        }
    }
    inline void createRenderPass ()
    {
    // Step 1: Prepare attachments info
        {
            attachments.resize(1);
        // Attachment 0 is color attachment
            auto& dsc = attachments[0];
            dsc.flags = 0;
            dsc.format = selectedSurfaceFormat.format;
            dsc.samples = VK_SAMPLE_COUNT_1_BIT;
            dsc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            dsc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            dsc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            dsc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // for automatic layout transform (e.g. if input attachment layout != initialLayout then trans it)
            dsc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            dsc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            colorAttachmentReferences.resize(1);
            for (uint32_t i = 0; i < colorAttachmentReferences.size(); ++i) {
                auto& ref = colorAttachmentReferences[i];
            // index in vector of attachments
                ref.attachment = i;
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
    // Step 2: Prepare subpasses info
        subpasses.resize(1);
        {
            auto& dsc = subpasses[0];
            dsc.flags = 0;
            dsc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            dsc.inputAttachmentCount = 0;
            dsc.pInputAttachments = nullptr;
            dsc.colorAttachmentCount = colorAttachmentReferences.size();
            dsc.pColorAttachments = colorAttachmentReferences.data();
            dsc.pResolveAttachments = nullptr;
            dsc.pDepthStencilAttachment = nullptr;
            dsc.preserveAttachmentCount = 0;
            dsc.pPreserveAttachments = nullptr;
        }
    // Step 3: Prepare subpass dependencies
        dependencies.resize(1);
        {
            auto& dep = dependencies[0];
            dep.srcSubpass = 0;
            dep.dstSubpass = 0;
            dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dep.srcAccessMask = 0;
            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT 
                              | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dep.dependencyFlags = 0;
        }
    // Step 4: Create render pass
        auto& ci = renderPassCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.attachmentCount = attachments.size();
        ci.pAttachments = attachments.data();
        ci.subpassCount = subpasses.size();
        ci.pSubpasses = subpasses.data();
        ci.dependencyCount = dependencies.size();
        ci.pDependencies = dependencies.data();
        VkResult r = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateRenderPass: {}", (int)r));
    }
    inline void readShaderCode (std::vector<char>& buffer, std::string rPath)
    {
        std::string absPath = cfg.spirvPath + std::string("/") + rPath;
        std::ifstream file(absPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open spirv file: " + absPath);
        }
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        buffer.resize(fileSize);
        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error("failed to read spirv file: " + absPath);
        }
        file.close();
    }
    inline void createShaderModule ()
    {
        shaderCodes.resize(2);
        readShaderCode(shaderCodes[0], "triangle.vert");
        readShaderCode(shaderCodes[1], "triangle.frag");
        shaderModuleCreateInfos.resize(shaderCodes.size());
        shaderModules.resize(shaderCodes.size());
        for (uint32_t i = 0; i < shaderModuleCreateInfos.size(); ++i) {
            auto& ci = shaderModuleCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.codeSize = shaderCodes[i].size();
            ci.pCode = reinterpret_cast<const uint32_t*>(shaderCodes[i].data());
            VkResult r = vkCreateShaderModule(device, &ci, nullptr, &(shaderModules[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateShaderModule: {}", (int)r));
        }
    }
// create infos about pipeline states
    inline void prepVertexInputStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.vertexInput;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.vertexBindingDescriptionCount = 0;
        ci.pVertexBindingDescriptions = nullptr;
        ci.vertexAttributeDescriptionCount = 0;
        ci.pVertexAttributeDescriptions = nullptr;
    }
    inline void prepInputAssemblyStateCreateInfo ()
    {
    // map infomation to assemble vertex to primitives (prim idx -> vert idx)
        auto& ci = pipelineStateCreateInfos.inputAssembly;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        ci.primitiveRestartEnable = VK_FALSE;
    }
    inline void prepTessellationStateCreateInfo ()
    {
        throw std::runtime_error("not implemented path");
        auto& ci = pipelineStateCreateInfos.tessellation;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.patchControlPoints = 0;
    }
    inline void prepViewportStateCreateInfo ()
    {
        {
            viewports.resize(1);
            auto& v = viewports[0];
            v.x = 0.0f;
            v.y = 0.0f;
            v.width = surfaceCap.currentExtent.width;
            v.height = surfaceCap.currentExtent.height;
            v.minDepth = 0.0f;
            v.maxDepth = 1.0f;
        }
        {
            scissors.resize(1);
            VkRect2D& s = scissors[0];
            s.offset = VkOffset2D {
                .x = 0u,
                .y = 0u
            };
            s.extent = surfaceCap.currentExtent;
        }
        auto& ci = pipelineStateCreateInfos.viewport;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.viewportCount = viewports.size();
        ci.pViewports = viewports.data();
        ci.scissorCount = scissors.size();
        ci.pScissors = scissors.data();
    }
    inline void prepRasterizationStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.rasterization;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.depthClampEnable = VK_FALSE;
        ci.rasterizerDiscardEnable = VK_FALSE;
    //  Other possible rasterization methods to draw a polygon are {
    //      VK_POLYGON_MODE_LINE,
    //      VK_POLYGON_MODE_POINT
    //  }
        ci.polygonMode = VK_POLYGON_MODE_FILL;
    // Cull back
        ci.cullMode = VK_CULL_MODE_BACK_BIT;
        ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
        ci.depthBiasEnable = VK_FALSE;
        ci.depthBiasConstantFactor = 0.0f;
        ci.depthBiasClamp = 0.0f;
        ci.depthBiasSlopeFactor = 0.0f;
        ci.lineWidth = 1.0f;
    }
    inline void prepMultisampleStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.multisample;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
    // sample count n means sample n times per pixel
        ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // ? run frag shader per sample for at least xx times (xx is next option)
        ci.sampleShadingEnable = VK_FALSE;
        ci.minSampleShading = 1.0f;
        ci.pSampleMask = nullptr;
    // see Multisample Coverage (https://docs.vulkan.org/spec/latest/chapters/fragops.html#fragops-covg)
        ci.alphaToCoverageEnable = VK_FALSE;
        ci.alphaToOneEnable = VK_FALSE;
    }
    inline void prepDepthStencilStateCreateInfo ()
    {
        throw std::runtime_error("not implemented path");
        auto& ci = pipelineStateCreateInfos.depthStencil;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
    }
    inline void prepColorBlendStateCreateInfo ()
    {
        {
            colorBlendAttachmentStates.resize(1);
            {
                auto& as = colorBlendAttachmentStates[0];
                as.blendEnable = VK_FALSE;
                as.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                as.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                as.colorBlendOp = VK_BLEND_OP_ADD;
                as.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                as.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                as.alphaBlendOp = VK_BLEND_OP_ADD;
                as.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                  | VK_COLOR_COMPONENT_G_BIT
                                  | VK_COLOR_COMPONENT_B_BIT
                                  | VK_COLOR_COMPONENT_A_BIT;
            }
        }
        auto& ci = pipelineStateCreateInfos.colorBlend;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.logicOpEnable = VK_FALSE;
        ci.logicOp = VK_LOGIC_OP_COPY;
        ci.attachmentCount = colorBlendAttachmentStates.size();
        ci.pAttachments = colorBlendAttachmentStates.data();
        for (size_t i = 0; i < 4; ++i) {
            ci.blendConstants[i] = 0.0f;
        }
    }
    inline void prepDynamicStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.dynamic;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.dynamicStateCount = dynamicStates.size();
        ci.pDynamicStates = dynamicStates.data();
    }
// end of pipeline states create info

    inline void createGraphicsPipeline ()
    {
    // Step 1: Prepare shader stages info
        shaderStageCreateInfos.resize(shaderModules.size());
        for (uint32_t i = 0; i < shaderStageCreateInfos.size(); ++i) {
            auto& ci = shaderStageCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            switch (i) {
                case 0u: ci.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
                case 1u: ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
                default: throw std::runtime_error("not implemented path");
            }
            ci.module = shaderModules[i];
            ci.pName = "main";
            ci.pSpecializationInfo = nullptr;
        }
    // Step 2: Prepare states info
        prepVertexInputStateCreateInfo();
        prepInputAssemblyStateCreateInfo();
        // prepTessellationStateCreateInfo();
        prepViewportStateCreateInfo();
        prepRasterizationStateCreateInfo();
        prepMultisampleStateCreateInfo();
        // prepDepthStencilStateCreateInfo();
        prepColorBlendStateCreateInfo();
        prepDynamicStateCreateInfo();
    // Step 3: Prepare descriptor set layout (not implemented yet)
    // Step 4: Create pipeline layout
        {
            auto& ci = pipelineLayoutCreateInfo;
            ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.setLayoutCount = descriptorSetLayout.size();
            ci.pSetLayouts = descriptorSetLayout.data();
            ci.pushConstantRangeCount = pushConstantRanges.size();
            ci.pPushConstantRanges = pushConstantRanges.data();VkResult r = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreatePipelineLayout: {}", (int)r));
        }
        auto& ci = graphicsPipelineCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.stageCount = shaderStageCreateInfos.size();
        ci.pStages = shaderStageCreateInfos.data();
        ci.pVertexInputState = &(pipelineStateCreateInfos.vertexInput);
        ci.pInputAssemblyState = &(pipelineStateCreateInfos.inputAssembly);
        // ci.pTessellationState = &(pipelineStateCreateInfos.tessellation);
        ci.pTessellationState = nullptr;
        ci.pViewportState = &(pipelineStateCreateInfos.viewport);
        ci.pRasterizationState = &(pipelineStateCreateInfos.rasterization);
        ci.pMultisampleState = &(pipelineStateCreateInfos.multisample);
        // ci.pDepthStencilState = &(pipelineStateCreateInfos.depthStencil);
        ci.pDepthStencilState = nullptr;
        ci.pColorBlendState = &(pipelineStateCreateInfos.colorBlend);
        ci.pDynamicState = &(pipelineStateCreateInfos.dynamic);
        ci.layout = pipelineLayout;
        ci.renderPass = renderPass;
        ci.subpass = 0;
        ci.basePipelineHandle = VK_NULL_HANDLE;
        ci.basePipelineIndex = 0;
        VkResult r = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateGraphicsPipelines: {}", (int)r));
    }
    inline void createFramebuffer ()
    {
        framebufferCreateInfos.resize(swapchainImageViews.size());
        framebuffers.resize(swapchainImageViews.size());
        for (uint32_t i = 0; i < framebufferCreateInfos.size(); ++i) {
            auto& ci = framebufferCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.renderPass = renderPass;
            ci.attachmentCount = 1;
            ci.pAttachments = &(swapchainImageViews[i]);
        // three dimensions of the framebuffer
            ci.width = surfaceCap.currentExtent.width;
            ci.height = surfaceCap.currentExtent.height;
            ci.layers = 1;
            VkResult r = vkCreateFramebuffer(device, &ci, nullptr, &(framebuffers[i]));
            if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreateFramebuffer: {}", (int)r));
        }
    }

    void buildCommandBuffer ();
    void buildGraphicsPipeline ();
    void buildSwapchain ();

public:
    Vulkan (std::vector<const char*> additionalInstanceExtensions, Config& cfg);
    ~Vulkan ();
    Vulkan (Vulkan& rhs) = delete;
    Vulkan (Vulkan&& rhs) = delete;

    void render ();
    
    inline VkInstance& getInstance ()
    {
        return instance;
    }
    
    inline void initGraphics (VkSurfaceKHR& s)
    {
        surface = s;
        buildSwapchain();
        buildGraphicsPipeline();
        buildCommandBuffer();
    }
};
#endif
