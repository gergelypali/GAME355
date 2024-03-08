#ifndef VULKANRENDERABLEOBJECT_H
#define VULKANRENDERABLEOBJECT_H

#include "Structs.h"
#include <vulkan/vulkan.h>

using pipelineInfo = PIPELINE::pipelineInfo;

class DeviceHandler;
class PipelineManager;

class VulkanRenderableObject
{
public:
    VulkanRenderableObject() = delete;
    VulkanRenderableObject(DeviceHandler* dh, PipelineManager* pm, const std::string& name) : m_dh(dh), m_pm(pm), m_name(name) {};
    virtual void updateUBO() = 0;
    virtual void resetFrameVariables() = 0;
    virtual void createPipeline() = 0;
    virtual void createCommandBuffer(VkCommandBuffer& buffer) = 0;

protected:
    virtual void init() = 0;
    virtual void createDescriptorCreateInfo() = 0;

    //variables
    std::string m_name{""};
    DeviceHandler* m_dh{nullptr};
    PipelineManager* m_pm{nullptr};

};

#endif