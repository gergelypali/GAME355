#ifndef SHAPE2D_H
#define SHAPE2D_H

#include "VulkanRenderableObject.h"
#include <map>
#include <unordered_map>

using shape2dUboData = BUFFER::shape2dUboData;

class Shape2d : public VulkanRenderableObject
{
public:
    Shape2d() : VulkanRenderableObject("shape2d") { init(); };
    void updateUBO() override;
    void resetFrameVariables() override;
    void createPipeline(PipelineManager* pm) override;
    void createCommandBuffer(VkCommandBuffer& buffer) override;
    void createUboBuffer(DeviceHandler* dh) override;

    void addShape2dToDraw(const std::string &nameVertex, const std::string &nameIndex, MATH::Vec4& positionAndSize, MATH::Vec4 &color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount);
    void addShape2dToDraw(const std::string &nameVertex, const std::string &nameIndex, const std::string &nameTexture, MATH::Vec4& positionAndSize, VkDescriptorSet& set, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount);

    size_t m_shapeCount{0};

private:
    void init() override;

    struct shapeData
    {
        std::vector<std::pair<MATH::Vec4, MATH::Vec4>> uboData;
        std::string nameVertex;
        std::string nameIndex;
        std::string nameTexture{""};
    };

    std::map<std::string, shapeData> m_vertexData;
    shape2dUboData m_ubodata;
    std::unordered_map<std::string, VkBuffer*> m_vertexBufferMap;
    std::unordered_map<std::string, std::pair<VkBuffer*, int>> m_indexBufferMap;
    std::unordered_map<std::string, VkDescriptorSet*> m_textureMap;

    VkDescriptorSet ubo0Set{VK_NULL_HANDLE};
    VkDescriptorSet sampler1Set{VK_NULL_HANDLE};
    VkPipeline ubo0Sampler1Pipeline{VK_NULL_HANDLE};
    VkPipelineLayout ubo0Sampler1Pipelinelayout{VK_NULL_HANDLE};
    VkPipeline ubo0Pipeline{VK_NULL_HANDLE};
    VkPipelineLayout ubo0Pipelinelayout{VK_NULL_HANDLE};

    VkBuffer uboBuffer{VK_NULL_HANDLE};
    VkDeviceMemory uboMemory{VK_NULL_HANDLE};
    void* uboAddress{nullptr};
};

#endif