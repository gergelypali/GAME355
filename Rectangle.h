#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "VulkanRenderableObject.h"

using rectangleUboData = BUFFER::rectangleUboData;

class DeviceHandler;
class PipelineManager;

class Rectangle : public VulkanRenderableObject
{
public:
    Rectangle() : VulkanRenderableObject("rectangle") { init(); };
    void updateUBO(void* address) override;
    void resetFrameVariables() override;
    void createPipeline(PipelineManager* pm) override;
    void createCommandBuffer(VkCommandBuffer& buffer) override;
    std::vector<descriptorCreateInfo> createDescriptorCreateInfo() override;

    size_t m_rectangleCount{0};
    rectangleUboData m_ubodata;

private:
    void init() override;

};

#endif