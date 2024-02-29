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

    gpuInfo m_info{};

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
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_renderFinishedFence;
    uint32_t m_currentImageIndex{0};
    VkRenderPass m_renderPass{};

    bool IsExtensionSupported(const std::vector<VkExtensionProperties>& supportedExtensions, const char* extension);

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createSwapchainImageViews();
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
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


public:
    DeviceHandler() = delete;
    DeviceHandler(const DeviceHandler&) = delete;
    DeviceHandler(DeviceHandler&&) = delete;
    DeviceHandler& operator=(const DeviceHandler&) = delete;
    DeviceHandler& operator=(DeviceHandler&&) = delete;

    DeviceHandler(SDL_Window* window);
    ~DeviceHandler();

    void checkVkResult(const VkResult& res);

    void drawFrame(VkCommandBuffer& buffer);
    void createCommandBuffer(VkCommandBuffer& buffer, VkCommandBufferLevel level);
    void createCommandBuffer(std::vector<VkCommandBuffer>& buffer, VkCommandBufferLevel level);
    void recordPrimaryCommandBuffer(VkCommandBuffer& buffer, std::vector<VkCommandBuffer>& secBuffers);
    void recordSecondaryCommandBufferStart(VkCommandBuffer& buffer, const VkPipeline& pipeline);
    void recordSecondaryCommandBufferEnd(VkCommandBuffer& buffer);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    SDL_Window* getWindow() { return m_window; };
    VkExtent2D &getWindowSize() { return m_info.physicalDeviceSurfaceCapabilities.currentExtent; };
    VkInstance getInstance() { return m_instance; };
    VkSurfaceKHR getSurface() { return m_surface; };
    VkDebugUtilsMessengerEXT getDebugMessenger() { return m_debugMessenger; };
    VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; };
    VkDevice &getLogicalDevice() { return m_logicalDevice; };
    VkQueue getGraphicsQueue() { return m_graphicsQueue; };
    VkQueue getPresentQueue() { return m_presentQueue; };
    VkRenderPass &getRenderPass() { return m_renderPass; };
};

#endif
