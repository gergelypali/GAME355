#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <vector>
#include <SDL.h>
#include <iostream>
#include <optional>
#include <vulkan/vulkan.h>
#include "Vector.h"

class DeviceHandler
{
private:
    struct gpuInfo
    {
        // data for later use
        std::optional<uint32_t> graphicsQueueIndex{};
        std::optional<uint32_t> presentQueueIndex{};
        VkSurfaceFormatKHR swapChainSurfaceFormat;
        VkPresentModeKHR swapChainPresentMode;
        uint32_t swapChainImageCount{0};

        // data for setup
        std::vector<const char*> instanceValidationLayers;
        std::vector<const char*> requiredPhysicalDeviceExtensions;
        std::vector<const char*> requiredInstanceExtensions;

        // queried data from the GPU
        std::vector<VkExtensionProperties> supportedInstanceExtensions;
        std::vector<VkPhysicalDevice> gpus;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties;
        std::vector<VkExtensionProperties> physicalDeviceExtensionProperties;
        VkSurfaceCapabilitiesKHR physicalDeviceSurfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats;
        std::vector<VkPresentModeKHR> physicalDevicePresentModes;
    };

    struct pipelineInfo
    {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        std::vector<VkDynamicState> dynamicStates;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        VkPipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo{};
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VkPipelineLayout pipelineLayout{};
        VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        VkPipeline pipeline;
    };

    gpuInfo m_info{};
    pipelineInfo m_baseGraphicsPipeline{};

    const int m_concurrentFrames{2};//not used yet; maybe later

    SDL_Window* m_window{nullptr};
    VkInstance m_instance{nullptr};
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_renderFinishedFence;
    uint32_t m_currentImageIndex{0};
    VkRenderPass m_renderPass{};

    //temp struct for a push constant
    struct pushConstant
    {
        MATH::Vec4 position;
    };

    void checkVkResult(const VkResult& res);
    bool IsExtensionSupported(const std::vector<VkExtensionProperties>& supportedExtensions, const char* extension);

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createSwapchainImageViews();
    std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::string& path);
    void createGraphicsPipeline();
    void createRenderPass();
    void createSwapchainFramebuffers();
    void createCommandPool();
    void recordCommand(VkCommandBuffer buffer, uint32_t imageIndex);
    void createSyncObjects();

    // debugging part
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
        );
    void setupDebugMessenger();
    void teardownDebugMessenger();

    // physical device helpers
    bool isDeviceGoodForUs(const VkPhysicalDevice& device);


public:
    DeviceHandler() = delete;
    DeviceHandler(const DeviceHandler&) = delete;
    DeviceHandler(DeviceHandler&&) = delete;
    DeviceHandler& operator=(const DeviceHandler&) = delete;
    DeviceHandler& operator=(DeviceHandler&&) = delete;

    DeviceHandler(SDL_Window* window);
    ~DeviceHandler();

    void drawFrame(VkCommandBuffer& buffer);
    void createCommandBuffer(VkCommandBuffer& buffer, VkCommandBufferLevel level);
    void createCommandBuffer(std::vector<VkCommandBuffer>& buffer, VkCommandBufferLevel level);
    void recordPrimaryCommandBuffer(VkCommandBuffer& buffer, std::vector<VkCommandBuffer>& secBuffers);
    void recordSecondaryCommandBufferStart(VkCommandBuffer& buffer);
    void recordSecondaryCommandBufferEnd(VkCommandBuffer& buffer);
    void sendPushConstant(VkCommandBuffer& buffer, MATH::Vec4& position);

    SDL_Window* getWindow() { return m_window; };
    VkInstance getInstance() { return m_instance; };
    VkSurfaceKHR getSurface() { return m_surface; };
    VkDebugUtilsMessengerEXT getDebugMessenger() { return m_debugMessenger; };
    VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; };
    VkDevice getLogicalDevice() { return m_logicalDevice; };
    VkQueue getGraphicsQueue() { return m_graphicsQueue; };
    VkQueue getPresentQueue() { return m_presentQueue; };
};

#endif
