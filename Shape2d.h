#ifndef SHAPE2D_H
#define SHAPE2D_H

#include "VulkanRenderableObject.h"
#include <map>
#include <unordered_map>

using shape2dUboData = BUFFER::shape2dUboData;

class DeviceHandler;
class PipelineManager;

class Shape2d : public VulkanRenderableObject
{
public:
    Shape2d() : VulkanRenderableObject("shape2d") { init(); };
    void updateUBO(void* address) override;
    void resetFrameVariables() override;
    void createPipeline(PipelineManager* pm) override;
    void createCommandBuffer(VkCommandBuffer& buffer) override;
    std::vector<descriptorCreateInfo> createDescriptorCreateInfo() override;

    void addShape2dToDraw(const std::string& name, MATH::Vec4& positionAndSize, MATH::Vec4 &color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount);

    size_t m_shapeCount{0};

private:
    void init() override;

    std::map<std::string, std::vector<std::pair<MATH::Vec4, MATH::Vec4>>> m_vertexData;
    shape2dUboData m_ubodata;
    std::unordered_map<std::string, VkBuffer> m_vertexBufferMap;
    std::unordered_map<std::string, std::pair<VkBuffer, int>> m_indexBufferMap;

};

#endif