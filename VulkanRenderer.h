#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <SDL.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "Vector.h"
#include <memory>
#include <map>

class DeviceHandler;
class PipelineManager;
class Entity;
class VulkanRenderableObject;

class VulkanRenderer
{
private:
    SDL_Window* m_window;
    uint32_t m_windowX{0};
    uint32_t m_windowY{0};

    DeviceHandler* m_deviceHandler{nullptr};
    PipelineManager* m_pipelineManager{nullptr};

    // we store the renderable objects in a vector maybe
    std::map<std::string, VulkanRenderableObject*> m_renderTheseObjects;
    //std::vector<VulkanRenderableObject*> m_renderTheseObjects;

    VkCommandBuffer m_primaryCommandBuffer;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffer;

    void createPrimaryCommandBuffer(VkCommandBuffer& buffer);
    void createSecondaryCommandBuffer(std::vector<VkCommandBuffer>& buffer);

    bool createAndCopyDataToGPUSideBuffer(VkBuffer& buffer, VkBufferUsageFlags flags, VkDeviceMemory& bufferMemory, VkDeviceSize size, void* dataPointer);
    bool createAndCopyDataToCPUSideBuffer(VkBuffer& buffer, VkBufferUsageFlags flags, VkDeviceMemory& bufferMemory, VkDeviceSize size, void* dataPointer);
    std::vector<MATH::Vec2> load2dVertexFile(const std::string& pathToFile);
    std::vector<uint32_t> loadIndexFile(const std::string& pathToFile);

public:
    VulkanRenderer() = delete;
    VulkanRenderer(const VulkanRenderer&) = delete;  /// Copy constructor
    VulkanRenderer(VulkanRenderer&&) = delete;       /// Move constructor
    VulkanRenderer& operator=(const VulkanRenderer&) = delete; /// Copy operator
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;      /// Move operator

    VulkanRenderer(SDL_Window* window);
    ~VulkanRenderer();

    void drawFrame();
    void vulkanRenderRect(const MATH::Vec2& position, const MATH::Vec2& size, const MATH::Vec4& color);//this will just update the command buffer with the new commands
    void vulkanRenderShape2d(const std::string& nameVertex, const std::string &nameIndex, const MATH::Vec2& position, const MATH::Vec2& size, MATH::Vec4& color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount);

    bool load2dVertexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    bool loadIndexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory, int& size);
};

#endif