#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "VulkanRenderableObject.h"

using rectangleUboData = BUFFER::rectangleUboData;

class DeviceHandler;
class PipelineManager;

class Rectangle : public VulkanRenderableObject
{
public:
    Rectangle() = delete;
    Rectangle(DeviceHandler* dh, PipelineManager* pm) : VulkanRenderableObject(dh, pm, "rectangle") { init(); };
    void updateUBO() override;
    void resetFrameVariables() override;
    void createPipeline() override;
    void createCommandBuffer(VkCommandBuffer& buffer) override;

    size_t m_rectangleCount{0};
    rectangleUboData m_ubodata;

private:
    void init() override;
    void createDescriptorCreateInfo() override;

};

#endif