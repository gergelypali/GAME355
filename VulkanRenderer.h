#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <SDL.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "Vector.h"

class DeviceHandler;
class PipelineManager;

class VulkanRenderer
{
private:
    SDL_Window* m_window;

    struct pushConstant
    {
        MATH::Vec4 position;
    };

    DeviceHandler* m_deviceHandler{nullptr};
    PipelineManager* m_pipelineManager{nullptr};

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