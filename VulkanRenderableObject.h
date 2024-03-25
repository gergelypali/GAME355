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
    VulkanRenderableObject(const std::string& name) : m_name(name) {};
    virtual void updateUBO() = 0;
    virtual void resetFrameVariables() = 0;
    virtual void createUboBuffer(DeviceHandler* dh) = 0;
    virtual void createPipeline(PipelineManager* pm) = 0;
    virtual void createCommandBuffer(VkCommandBuffer& buffer) = 0;

    std::string &getName() { return m_name; };

protected:
    virtual void init() = 0;

    //variables
    std::string m_name{""};

};

#endif