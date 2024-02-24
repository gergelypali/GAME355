#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <SDL.h>
#include <vulkan/vulkan.h>
#include <vector>

class DeviceHandler;

class VulkanRenderer
{
private:
    SDL_Window* m_window;

    DeviceHandler* m_deviceHandler{nullptr};

    VkCommandBuffer m_primaryCommandBuffer;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffer;

    void createPrimaryCommandBuffer(VkCommandBuffer& buffer);
    void createSecondaryCommandBuffer(std::vector<VkCommandBuffer>& buffer);

public:
    VulkanRenderer() = delete;
    VulkanRenderer(const VulkanRenderer&) = delete;  /// Copy constructor
    VulkanRenderer(VulkanRenderer&&) = delete;       /// Move constructor
    VulkanRenderer& operator=(const VulkanRenderer&) = delete; /// Copy operator
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;      /// Move operator

    VulkanRenderer(SDL_Window* window);
    ~VulkanRenderer();

    void drawFrame();
    void vulkanRenderRect();//this will just update the command buffer with the new commands
};

#endif