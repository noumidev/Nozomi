/*
    Nozomi is an experimental HLE Switch emulator.
    Copyright (C) 2023  noumidev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "renderer.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <optional>
#include <set>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <plog/Log.h>

#include "file.hpp"
#include "types.hpp"
#include "window.hpp"

namespace renderer {

using vec2 = float[2];
using vec3 = float[3];

constexpr bool ENABLE_VALIDATION_LAYERS = true;

const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};

struct Vertex {
    vec2 pos;
    vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
} __attribute__((packed));

const std::vector<Vertex> VERTICES = {
    {{-1, -1}, {0.80, 0.80, 1.0}},
    {{ 1, -1}, {0.25, 0.25, 1.0}},
    {{ 1,  1}, {0.50, 1.00, 1.0}},
    {{-1,  1}, {0.60, 0.90, 1.0}},
};

const std::vector<u16> INDICES = {
    0, 1, 2, 2, 3, 0,
};

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily, presentFamily;

    bool hasAll() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;

    bool hasAll() {
        return !surfaceFormats.empty() && !presentModes.empty();
    }
};

struct RendererState {
    VkInstance instance;

    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue, presentQueue;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    VkRenderPass renderPass;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkSemaphore imageAvailableSema, renderFinishedSema;
    VkFence inFlightFence;

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
};

RendererState state;

// Returns true if all requested validation layers are supported
bool validationLayersSupported() {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    // Get available validation layers
    std::vector<VkLayerProperties> properties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, properties.data());

    PLOG_INFO << "Available validation layers:";
    for (const VkLayerProperties &property : properties) {
        PLOG_INFO << property.layerName;
    }

    for (const char *layerName : VALIDATION_LAYERS) {
        bool layerFound = false;

        for (const VkLayerProperties &property : properties) {
            if (std::strcmp(layerName, property.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

bool deviceExtensionsSupported(VkPhysicalDevice device) {
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, extensions.data());

    PLOG_INFO << "Available extensions:";
    for (const VkExtensionProperties &extensionProperties : extensions) {
        PLOG_INFO << extensionProperties.extensionName;
    }

    for (const char *extensionName : DEVICE_EXTENSIONS) {
        bool extensionFound = false;

        for (const VkExtensionProperties &extensionProperties : extensions) {
            if (std::strcmp(extensionName, extensionProperties.extensionName) != 0) {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            return false;
        }
    }

    return true;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    u32 queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    // Get queue families
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (u32 i = 0; i < queueFamilyCount; i++) {
        if (indices.hasAll()) {
            break;
        }

        if (!indices.graphicsFamily.has_value() && ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)) {
            indices.graphicsFamily = i;
        }

        if (!indices.presentFamily.has_value()) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, state.surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }
        }
    }

    return indices;
}

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, state.surface, &details.capabilities);

    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.surface, &formatCount, NULL);

    // Get available formats
    details.surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.surface, &formatCount, details.surfaceFormats.data());

    u32 presentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.surface, &presentModesCount, NULL);

    // Get available present modes
    details.presentModes.resize(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.surface, &presentModesCount, details.presentModes.data());

    return details;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    const bool extensionsSupported = deviceExtensionsSupported(device);

    bool swapChainSuitable = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapChainSupport = querySwapchainSupport(device);

        swapChainSuitable = swapChainSupport.hasAll();
    }

    return indices.hasAll() && extensionsSupported && swapChainSuitable;
}

VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const VkSurfaceFormatKHR &surfaceFormat : availableFormats) {
        if ((surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) && (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
            return surfaceFormat;
        }
    }

    return availableFormats[0];
}

VkExtent2D selectExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window::getWindow(), &width, &height);

        VkExtent2D actualExtent = {
            (u32)width,
            (u32)height,
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkShaderModule makeShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    shaderModuleCreateInfo.pCode = (u32 *)code.data();
    shaderModuleCreateInfo.codeSize = code.size();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(state.device, &shaderModuleCreateInfo, NULL, &shaderModule) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create shader module";

        exit(0);
    }

    return shaderModule;
}

u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(state.physicalDevice, &properties);

    for (u32 i = 0; i < properties.memoryTypeCount; i++) {
        if (((typeFilter & (1 << i)) != 0) && ((properties.memoryTypes[i].propertyFlags & flags) == flags)) {
            return i;
        }
    }

    PLOG_FATAL << "Failed to find suitable memory type";

    exit(0);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(state.device, &bufferCreateInfo, NULL, &buffer) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create buffer";

        exit(0);
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(state.device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, flags);

    if (vkAllocateMemory(state.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to allocate buffer memory";

        exit(0);
    }

    vkBindBufferMemory(state.device, buffer, bufferMemory, 0);
}

void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = state.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(state.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};

    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;

    vkQueueSubmit(state.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(state.graphicsQueue);

    vkFreeCommandBuffers(state.device, state.commandPool, 1, &commandBuffer);
}

void makeInstance() {
    // Enable Vulkan validation layers
    if constexpr (ENABLE_VALIDATION_LAYERS) {
        if (!validationLayersSupported()) {
            PLOG_FATAL << "Requested validation layers are unsupported";

            exit(0);
        }
    }

    // Make VkApplicationInfo
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    applicationInfo.pApplicationName = "Nozomi";
    applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    applicationInfo.pEngineName = "NozomiEngine";
    applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

    // Make VkInstanceCreateInfo
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    // Figure out how many Vulkan extensions are required by GLFW
    u32 glfwExtensionCount;

    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> enabledExtensions;
    for (u32 i = 0; i < glfwExtensionCount; i++) {
        enabledExtensions.push_back(glfwExtensions[i]);
    }

    #if defined(__APPLE__)
        enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif

    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceCreateInfo.enabledExtensionCount = (u32)enabledExtensions.size();

    if constexpr (ENABLE_VALIDATION_LAYERS) {
        instanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        instanceCreateInfo.enabledLayerCount = (u32)VALIDATION_LAYERS.size();
    }

    if (vkCreateInstance(&instanceCreateInfo, NULL, &state.instance) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create Vulkan instance";

        exit(0);
    }
}

void makeSurface() {
    if (glfwCreateWindowSurface(state.instance, window::getWindow(), NULL, &state.surface) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create surface";

        exit(0);
    }
}

void selectPhysicalDevice() {
    u32 deviceCount;
    vkEnumeratePhysicalDevices(state.instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        PLOG_FATAL << "Failed to find physical devices";

        exit(0);
    }

    // Get all available devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(state.instance, &deviceCount, devices.data());

    PLOG_INFO << "Physical devices:";
    for (const VkPhysicalDevice &device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        PLOG_INFO << deviceProperties.deviceName;

        if ((state.physicalDevice == VK_NULL_HANDLE) && isDeviceSuitable(device)) {
            state.physicalDevice = device;
        }
    }

    if (state.physicalDevice == VK_NULL_HANDLE) {
        PLOG_FATAL << "Failed to find suitable physical device";

        exit(0);
    }
}

void makeLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(state.physicalDevice);

    const std::set<u32> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Create queue infos
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    const float queuePriority = 1.0;
    for (u32 queueFamily : uniqueQueueFamilies) {
        // Make VkDeviceQueueCreateInfo
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        queueCreateInfo.queueCount = 1;

        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{}; // Unused for now

    // Make VkDeviceCreateInfo
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();

    // For compatibility with older Vulkan implementations
    deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    deviceCreateInfo.enabledExtensionCount = (u32)DEVICE_EXTENSIONS.size();

    if constexpr (ENABLE_VALIDATION_LAYERS) {
        deviceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        deviceCreateInfo.enabledLayerCount = (u32)VALIDATION_LAYERS.size();
    }

    if (vkCreateDevice(state.physicalDevice, &deviceCreateInfo, NULL, &state.device) != VK_SUCCESS) {
        PLOG_FATAL << "Unable to create logical device";

        exit(0);
    }

    vkGetDeviceQueue(state.device, indices.graphicsFamily.value(), 0, &state.graphicsQueue);
    vkGetDeviceQueue(state.device, indices.presentFamily.value(), 0, &state.presentQueue);
}

void makeSwapchain() {
    const SwapchainSupportDetails details = querySwapchainSupport(state.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(details.surfaceFormats);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = selectExtent(details.capabilities);

    PLOG_VERBOSE << "Extent = [" << extent.width << ", " << extent.height << "]";

    u32 imageCount = details.capabilities.minImageCount;
    if ((details.capabilities.maxImageCount > 0) && ((imageCount + 1) <= details.capabilities.maxImageCount)) {
        imageCount++;
    }

    // Make VkSwapchainCreateInfoKHR
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    swapchainCreateInfo.surface = state.surface;

    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageArrayLayers = 1;

    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;

    swapchainCreateInfo.imageExtent = extent;

    QueueFamilyIndices indices = findQueueFamilies(state.physicalDevice);
    u32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = details.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(state.device, &swapchainCreateInfo, NULL, &state.swapchain) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create swap chain";

        exit(0);
    }

    vkGetSwapchainImagesKHR(state.device, state.swapchain, &imageCount, NULL);

    // Get swap chain images
    state.swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(state.device, state.swapchain, &imageCount, state.swapchainImages.data());

    state.swapchainImageFormat = surfaceFormat.format;
    state.swapchainExtent = extent;
}

void makeImageViews() {
    const std::vector<VkImage> &swapchainImages = state.swapchainImages;

    std::vector<VkImageView> &swapchainImageViews = state.swapchainImageViews;

    swapchainImageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.format = state.swapchainImageFormat;

        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(state.device, &imageViewCreateInfo, NULL, &swapchainImageViews[i]) != VK_SUCCESS) {
            PLOG_FATAL << "Failed to create image view";

            exit(0);
        }
    }
}

void destroyImageViews() {
    for (VkImageView imageView : state.swapchainImageViews) {
        vkDestroyImageView(state.device, imageView, NULL);
    }
}

void makeRenderPass() {
    VkAttachmentDescription colorAttachment{};

    colorAttachment.format = state.swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.colorAttachmentCount = 1;

    VkSubpassDependency dependency{};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.attachmentCount = 1;

    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.subpassCount = 1;

    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.dependencyCount = 1;

    if (vkCreateRenderPass(state.device, &renderPassCreateInfo, NULL, &state.renderPass) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create render pass";

        exit(0);
    }
}

void makeGraphicsPipeline() {
    VkShaderModule vertShaderModule = makeShaderModule(readFile("src/renderer/shaders/vert.spv"));
    VkShaderModule fragShaderModule = makeShaderModule(readFile("src/renderer/shaders/frag.spv"));

    // Create vertex shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";
    vertShaderStageCreateInfo.pSpecializationInfo = NULL;

    // Create fragment shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";
    fragShaderStageCreateInfo.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    const VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    const std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;

    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};

    viewport.x = viewport.y = 0.0;

    viewport.width = (float)state.swapchainExtent.width;
    viewport.height = (float)state.swapchainExtent.height;

    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    VkRect2D scissor{};

    scissor.offset = {0, 0};
    scissor.extent = state.swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.viewportCount = 1;

    viewportStateCreateInfo.pScissors = &scissor;
    viewportStateCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0;

    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    colorBlendState.logicOpEnable = VK_FALSE;

    colorBlendState.pAttachments = &colorBlendAttachment;
    colorBlendState.attachmentCount = 1;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutCreateInfo.setLayoutCount = 0;

    if (vkCreatePipelineLayout(state.device, &pipelineLayoutCreateInfo, NULL, &state.pipelineLayout) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create pipeline layout";

        exit(0);
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    pipelineCreateInfo.pStages = shaderStageCreateInfos;
    pipelineCreateInfo.stageCount = 2;

    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDepthStencilState = NULL;
    pipelineCreateInfo.pDynamicState = NULL;

    pipelineCreateInfo.layout = state.pipelineLayout;
    pipelineCreateInfo.renderPass = state.renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(state.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &state.graphicsPipeline) != VK_SUCCESS) {
        std::puts("Failed to create graphics pipeline");

        exit(0);
    }

    vkDestroyShaderModule(state.device, vertShaderModule, NULL);
    vkDestroyShaderModule(state.device, fragShaderModule, NULL);
}

void makeFramebuffers() {
    const std::vector<VkImageView> &swapchainImageViews = state.swapchainImageViews;

    std::vector<VkFramebuffer> &swapchainFramebuffers = state.swapchainFramebuffers;

    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapchainImageViews[i],
        };

        VkFramebufferCreateInfo framebuffer{};
        framebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        framebuffer.pAttachments = attachments;
        framebuffer.attachmentCount = 1;
    
        framebuffer.renderPass = state.renderPass;

        framebuffer.width = state.swapchainExtent.width;
        framebuffer.height = state.swapchainExtent.height;
        framebuffer.layers = 1;

        if (vkCreateFramebuffer(state.device, &framebuffer, NULL, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            PLOG_FATAL << "Failed to create framebuffer";

            exit(0);
        }
    }
}

void destroyFramebuffers() {
    for (VkFramebuffer framebuffer : state.swapchainFramebuffers) {
        vkDestroyFramebuffer(state.device, framebuffer, NULL);
    }
}

void makeCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(state.physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(state.device, &commandPoolCreateInfo, NULL, &state.commandPool) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to create command pool";

        exit(0);
    }
}

void makeVertexBuffer() {
    VkDeviceSize size = sizeof(VERTICES[0]) * VERTICES.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // Write vertex buffer
    void *data;
    vkMapMemory(state.device, stagingBufferMemory, 0, size, 0, &data);
    std::memcpy(data, VERTICES.data(), size);
    vkUnmapMemory(state.device, stagingBufferMemory);

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, state.vertexBuffer, state.vertexBufferMemory);
    copyBuffer(stagingBuffer, state.vertexBuffer, size);

    vkDestroyBuffer(state.device, stagingBuffer, NULL);
    vkFreeMemory(state.device, stagingBufferMemory, NULL);
}

void makeIndexBuffer() {
    VkDeviceSize size = sizeof(INDICES[0]) * INDICES.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // Write index buffer
    void *data;
    vkMapMemory(state.device, stagingBufferMemory, 0, size, 0, &data);
    std::memcpy(data, INDICES.data(), size);
    vkUnmapMemory(state.device, stagingBufferMemory);

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, state.indexBuffer, state.indexBufferMemory);
    copyBuffer(stagingBuffer, state.indexBuffer, size);

    vkDestroyBuffer(state.device, stagingBuffer, NULL);
    vkFreeMemory(state.device, stagingBufferMemory, NULL);
}

void makeCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    commandBufferAllocateInfo.commandPool = state.commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(state.device, &commandBufferAllocateInfo, &state.commandBuffer) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to allocate command buffer(s)";

        exit(0);
    }
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to begin command buffer";

        exit(0);
    }

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassBeginInfo.renderPass = state.renderPass;
    renderPassBeginInfo.framebuffer = state.swapchainFramebuffers[imageIndex];

    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = state.swapchainExtent;

    VkClearValue clearColor = {{{0.0, 0.0, 0.0, 1.0}}};

    renderPassBeginInfo.pClearValues = &clearColor;
    renderPassBeginInfo.clearValueCount = 1;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline);

    VkBuffer vertexBuffers[] = {state.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, state.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, (u32)INDICES.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to record command buffer";

        exit(0);
    }
}

void makeSyncObjects() {
    VkSemaphoreCreateInfo semaCreateInfo{};
    semaCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if ((vkCreateSemaphore(state.device, &semaCreateInfo, NULL, &state.imageAvailableSema) != VK_SUCCESS) || (vkCreateSemaphore(state.device, &semaCreateInfo, NULL, &state.renderFinishedSema) != VK_SUCCESS) || (vkCreateFence(state.device, &fenceCreateInfo, NULL, &state.inFlightFence) != VK_SUCCESS)) {
        PLOG_FATAL << "Failed to create sync objects";

        exit(0);
    }
}

void destroySyncObjects() {
    vkDestroySemaphore(state.device, state.imageAvailableSema, NULL);
    vkDestroySemaphore(state.device, state.renderFinishedSema, NULL);
    vkDestroyFence(state.device, state.inFlightFence, NULL);
}

void init() {
    state.physicalDevice = VK_NULL_HANDLE;

    makeInstance();
    makeSurface();
    selectPhysicalDevice();
    makeLogicalDevice();
    makeSwapchain();
    makeImageViews();
    makeRenderPass();
    makeGraphicsPipeline();
    makeFramebuffers();
    makeCommandPool();
    makeVertexBuffer();
    makeIndexBuffer();
    makeCommandBuffer();
    makeSyncObjects();
}

void deinit() {
    destroySyncObjects();
    vkDestroyBuffer(state.device, state.indexBuffer, NULL);
    vkFreeMemory(state.device, state.indexBufferMemory, NULL);
    vkDestroyBuffer(state.device, state.vertexBuffer, NULL);
    vkFreeMemory(state.device, state.vertexBufferMemory, NULL);
    vkDestroyCommandPool(state.device, state.commandPool, NULL);
    destroyFramebuffers();
    vkDestroyPipeline(state.device, state.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(state.device, state.pipelineLayout, NULL);
    vkDestroyRenderPass(state.device, state.renderPass, NULL);
    destroyImageViews();
    vkDestroySwapchainKHR(state.device, state.swapchain, NULL);
    vkDestroyDevice(state.device, NULL);
    vkDestroySurfaceKHR(state.instance, state.surface, NULL);
    vkDestroyInstance(state.instance, NULL);
}

void draw() {
    vkWaitForFences(state.device, 1, &state.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(state.device, 1, &state.inFlightFence);

    u32 imageIndex;
    vkAcquireNextImageKHR(state.device, state.swapchain, UINT64_MAX, state.imageAvailableSema, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(state.commandBuffer, 0);
    recordCommandBuffer(state.commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {state.imageAvailableSema};
    VkSemaphore signalSemaphores[] = {state.renderFinishedSema};

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.pCommandBuffers = &state.commandBuffer;
    submitInfo.commandBufferCount = 1;

    if (vkQueueSubmit(state.graphicsQueue, 1, &submitInfo, state.inFlightFence) != VK_SUCCESS) {
        PLOG_FATAL << "Failed to submit command buffer";

        exit(0);
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.waitSemaphoreCount = 1;

    VkSwapchainKHR swapchains[] = {state.swapchain};

    presentInfo.pSwapchains = swapchains;
    presentInfo.swapchainCount = 1;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(state.presentQueue, &presentInfo);
}

void waitIdle() {
    vkDeviceWaitIdle(state.device);
}

}
