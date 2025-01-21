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
    uint32_t queueFamilyCount;
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

    struct {
        std::unique_ptr<std::vector<char>> vert = nullptr;
        std::unique_ptr<std::vector<char>> frag = nullptr;
    } shaderCodes;
    struct {
        VkShaderModuleCreateInfo vert;
        VkShaderModuleCreateInfo frag;
    } shaderModuleCreateInfos;
    struct {
        VkShaderModule vert; 
        VkShaderModule frag;
    } shaderModules;

    struct {
        VkPipelineShaderStageCreateInfo vert;
        VkPipelineShaderStageCreateInfo frag;
        uint32_t count = 2;
    } shaderStageCreateInfos;
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
    VkSemaphoreCreateInfo semaphoreCreateInfo;
    VkFenceCreateInfo fenceCreateInfo;
    std::vector<VkFence> inFlightFences;

    VkPipelineStageFlags waitDstStageMask;
    VkSubmitInfo submitInfo;
    VkPresentInfoKHR presentInfo;

//    VkBufferCreateInfo vertexBufferCreateInfo;

    inline void prepApplicationInfo() {
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = "Giftland";
        applicationInfo.applicationVersion = 1;
        applicationInfo.pEngineName = "Rxon";
        applicationInfo.engineVersion = 1;
        applicationInfo.apiVersion = VK_API_VERSION_1_0;
    }
    inline void prepInstanceCreateInfo() {
        auto& ici = instanceCreateInfo;
        ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ici.pNext = nullptr;
        ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        prepApplicationInfo();
        ici.pApplicationInfo = &applicationInfo;
        // TODO: Support enabling layers
        ici.enabledLayerCount = 0;
        ici.ppEnabledLayerNames = {};
        ici.enabledExtensionCount = instanceEnabledExtensionNames.size();
        ici.ppEnabledExtensionNames = instanceEnabledExtensionNames.data();
    }
    inline void getQueueFamily () 
    {
        vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCount, nullptr);
        queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    }
    inline void prepDeviceQueueCreateInfos ()
    {
        getQueueFamily();
        queueCreateInfos.resize(queueFamilyCount);
        queueFamilyPriorities.resize(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            auto& qci = queueCreateInfos[i];
            qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qci.pNext = nullptr;
            qci.flags = 0;
            qci.queueFamilyIndex = i;
            qci.queueCount = queueFamilyProperties[i].queueCount;
            queueFamilyPriorities[i] = std::vector<float>(queueFamilyProperties[i].queueCount, 0.5);
            qci.pQueuePriorities = queueFamilyPriorities[i].data();
        }
    }
    inline void prepDeviceCreateInfo ()
    {
        auto& dci = deviceCreateInfo;
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.pNext = nullptr;
        dci.flags = 0;
        prepDeviceQueueCreateInfos();
        assert(queueFamilyCount == queueCreateInfos.size());
        dci.queueCreateInfoCount = queueFamilyCount;
        dci.pQueueCreateInfos = queueCreateInfos.data();
    // TODO: Support enabling layers
        dci.enabledLayerCount = 0;
        dci.ppEnabledLayerNames = nullptr;
        dci.enabledExtensionCount = deviceEnabledExtensionNames.size();
        dci.ppEnabledExtensionNames = deviceEnabledExtensionNames.data();
        vkGetPhysicalDeviceFeatures(selectedPhysicalDevice, &physicalDeviceFeatures);
        dci.pEnabledFeatures = &physicalDeviceFeatures;
    }
    inline void prepSwapchainCreateInfo ()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedPhysicalDevice, surface, &surfaceCap);

        auto& sci = swapchainCreateInfo;
        sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        sci.pNext = nullptr;
        sci.flags = 0;
        sci.surface = surface;
        sci.minImageCount = std::max(surfaceCap.minImageCount, 2u);
        selectFormat();
        sci.imageFormat = selectedSurfaceFormat.format;
        sci.imageColorSpace = selectedSurfaceFormat.colorSpace;
        sci.imageExtent = surfaceCap.currentExtent;
    // non-stereoscopic-3D applications
        sci.imageArrayLayers = 1;
    // what you may use the swapchain as
        sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // which queue family first use the swapchain get the ownership
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sci.queueFamilyIndexCount = queueFamilyInUse.size();
        sci.pQueueFamilyIndices = queueFamilyInUse.data();
    // transforms like rotate, flip, etc
        sci.preTransform = surfaceCap.currentTransform;
    // to be controlled by windowing system, use VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // "present" means the process that the image passed after renderred
        sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    // are pixels removed because not seen or some other reasons
        sci.clipped = VK_TRUE;
        sci.oldSwapchain = VK_NULL_HANDLE;
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
    inline void prepSwapchainImageViewCreateInfo ()
    {
        swapchainImageViewCreateInfos.resize(swapchainImages.size());
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
        }
    }
    inline void prepAttachments ()
    {
        attachments.resize(1);
    // Attachment 0 is color attachment
        {
            auto& dsc = attachments[0];
            dsc.flags = 0;
            dsc.format = selectedSurfaceFormat.format;
            dsc.samples = VK_SAMPLE_COUNT_1_BIT;
            dsc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            dsc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            dsc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            dsc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            dsc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            dsc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        colorAttachmentReferences.resize(1);
        for (uint32_t i = 0; i < colorAttachmentReferences.size(); ++i) {
            auto& ref = colorAttachmentReferences[i];
        // index in vector<attachments>
            ref.attachment = i;
            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }
    inline void prepSubpasses ()
    {
        subpasses.resize(1);
        for (auto& dsc : subpasses) {
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
    }
    inline void prepDependencies ()
    {
        dependencies.resize(1);
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
    inline void prepRenderPassCreateInfo ()
    {
        auto& ci = renderPassCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        prepAttachments();
        ci.attachmentCount = attachments.size();
        ci.pAttachments = attachments.data();
        prepSubpasses();
        ci.subpassCount = subpasses.size();
        ci.pSubpasses = subpasses.data();
        prepDependencies();
        ci.dependencyCount = dependencies.size();
        ci.pDependencies = dependencies.data();
    }
    inline std::vector<char>& readShaderCode (std::string rPath)
    {
        std::string absPath = cfg.spirvPath + std::string("/") + rPath;
        std::ifstream file(absPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open spirv file: " + absPath);
        }
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        auto& buffer = *new std::vector<char>;
        buffer.resize(fileSize);
        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error("failed to read spirv file: " + absPath);
        }
        file.close();
        return buffer;
    }
    inline void prepShaderModuleCreateInfo ()
    {
        auto& civ = shaderModuleCreateInfos.vert;
        civ.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        civ.pNext = nullptr;
        civ.flags = 0;
        shaderCodes.vert.reset(&readShaderCode("triangle.vert"));
        civ.codeSize = shaderCodes.vert->size();
        civ.pCode = reinterpret_cast<const uint32_t*>(shaderCodes.vert->data());
        auto& cif = shaderModuleCreateInfos.frag;
        cif.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        cif.pNext = nullptr;
        cif.flags = 0;
        shaderCodes.frag.reset(&readShaderCode("triangle.frag"));
        cif.codeSize = shaderCodes.frag->size();
        cif.pCode = reinterpret_cast<const uint32_t*>(shaderCodes.frag->data());
    }
    inline void prepShaderStageCreateInfo ()
    {
        auto& civ = shaderStageCreateInfos.vert;
        civ.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        civ.pNext = nullptr;
        civ.flags = 0;
        civ.stage = VK_SHADER_STAGE_VERTEX_BIT;
        civ.module = shaderModules.vert;
        civ.pName = "main";
        civ.pSpecializationInfo = nullptr;
        auto& cif = shaderStageCreateInfos.frag;
        cif.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cif.pNext = nullptr;
        cif.flags = 0;
        cif.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        cif.module = shaderModules.frag;
        cif.pName = "main";
        cif.pSpecializationInfo = nullptr;
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
        inline void prepViewports ()
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
        inline void prepScissors ()
        {
            scissors.resize(1);
            VkRect2D& s = scissors[0];
            s.offset = VkOffset2D {
                .x = 0u,
                .y = 0u
            };
            s.extent = surfaceCap.currentExtent;
        }
    inline void prepViewportStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.viewport;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        prepViewports();
        ci.viewportCount = viewports.size();
        ci.pViewports = viewports.data();
        prepScissors();
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
    //  other possible rasterization methods to draw a polygon are {
    //      VK_POLYGON_MODE_LINE,
    //      VK_POLYGON_MODE_POINT
    //  }
        ci.polygonMode = VK_POLYGON_MODE_FILL;
    // cull back
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
        inline void prepColorBlendAttachmentStates ()
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
    inline void prepColorBlendStateCreateInfo ()
    {
        auto& ci = pipelineStateCreateInfos.colorBlend;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.logicOpEnable = VK_FALSE;
        ci.logicOp = VK_LOGIC_OP_COPY;
        prepColorBlendAttachmentStates();
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
    inline void prepPipelineLayoutCreateInfo ()
    {
        auto& ci = pipelineLayoutCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.setLayoutCount = descriptorSetLayout.size();
        ci.pSetLayouts = descriptorSetLayout.data();
        ci.pushConstantRangeCount = pushConstantRanges.size();
        ci.pPushConstantRanges = pushConstantRanges.data();
    }
    inline void createPipelineLayout ()
    {
        prepPipelineLayoutCreateInfo();
        VkResult r = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
        if (r != VK_SUCCESS) throw std::runtime_error(std::format("vkCreatePipelineLayout: {}", (int)r));
    }
    inline void prepGraphicsPipelineCreateInfo ()
    {
        auto& ci = graphicsPipelineCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.stageCount = shaderStageCreateInfos.count;
        //ci.pStages = (const VkPipelineShaderStageCreateInfo*)&shaderStageCreateInfos;
        ci.pStages = &shaderStageCreateInfos.vert;
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
        createPipelineLayout();
        ci.layout = pipelineLayout;
        ci.renderPass = renderPass;
        ci.subpass = 0;
        ci.basePipelineHandle = VK_NULL_HANDLE;
        ci.basePipelineIndex = 0;
    }
    inline void prepFramebufferCreateInfo ()
    {
        auto& cis = framebufferCreateInfos;
        cis.resize(swapchainImageViews.size());
        for (uint32_t i = 0; i < cis.size(); ++i) {
            auto& ci = cis[i];
            ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.renderPass = renderPass;
            ci.attachmentCount = 1;
            ci.pAttachments = &(swapchainImageViews[i]);
        // 3 dimensions of the framebuffer
            ci.width = surfaceCap.currentExtent.width;
            ci.height = surfaceCap.currentExtent.height;
            ci.layers = 1;
        }
    }
    inline void prepCommandPoolCreateInfo ()
    {
        auto& cis = commandPoolCreateInfos;
        cis.resize(queueFamilyInUse.size());
        for (uint32_t i = 0; i < commandPoolCreateInfos.size(); ++i) {
            auto& ci = commandPoolCreateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.flags = queueFamilyInUse[i];
        }
    }
    inline void prepCommandBufferAllocateInfo ()
    {
        auto& cis = commandBufferAllocateInfos;
        cis.resize(commandPools.size());
        for (uint32_t i = 0; i < commandBufferAllocateInfos.size(); ++i) {
            auto& ci = commandBufferAllocateInfos[i];
            ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            ci.pNext = nullptr;
            ci.commandPool = commandPools[i];
            ci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            ci.commandBufferCount = framebuffers.size();   
        }
    }
    inline void prepCommandBufferBeginInfo ()
    {
        auto& ci = commandBufferBeginInfo;
        ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        ci.pNext = nullptr;
        ci.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        ci.pInheritanceInfo = nullptr;
    }
        inline void prepAttachmentClearValues ()
        {
            attachmentClearValues.resize(attachments.size());
            for (auto& el : attachmentClearValues) {
                for (size_t i = 0; i < 4; ++i) {
                    el.color.float32[i] = 0.0f;
                }
            }
        }
    inline void prepRenderPassBeginInfo ()
    {
        auto& cis = renderPassBeginInfos;
        cis.resize(framebuffers.size());
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
            prepAttachmentClearValues();
            ci.clearValueCount = attachmentClearValues.size();
            ci.pClearValues = attachmentClearValues.data();
        }
    }
    inline void prepSemaphoreCreateInfo ()
    {
        auto& ci = semaphoreCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
    }
    inline void prepFenceCreateInfo ()
    {
        auto& ci = fenceCreateInfo;
        ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        ci.pNext = nullptr;
    // created in signaled state
        ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    inline void prepSubmitInfo (uint32_t queueFamilyIndex, uint32_t currentFrame, uint32_t imageIndex)
    {
        auto& si = submitInfo;
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.pNext = nullptr;
        si.waitSemaphoreCount = 1;
        si.pWaitSemaphores = &(imageAvailableSemaphores[currentFrame]);
        waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        si.pWaitDstStageMask = &waitDstStageMask;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &commandBuffers[queueFamilyIndex][imageIndex];
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = &renderFinishedSemaphores[imageIndex];
    }
    inline void prepPresentInfo (uint32_t currentFrame, uint32_t& imageIndex)
    {
        auto& pi = presentInfo;
        pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        pi.pNext = nullptr;
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &(renderFinishedSemaphores[currentFrame]);
        pi.swapchainCount = 1;
        pi.pSwapchains = &swapchain;
        pi.pImageIndices = &imageIndex;
        pi.pResults = nullptr;
    }

    void createInstance ();
    void selectPhysicalDevice ();
    void createDevice ();
    void getDeviceQueues ();
    void selectFormat ();
    void createShaderModule ();

    void createSwapchainImageView ();
    void createRenderPass ();
    void createFramebuffer ();
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
