#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <map>
#include <string>
#include <vulkan/vulkan.h>
#include <functional>
#include "Vector.h"
#include "Structs.h"

using vec4PC = BUFFER::vec4PushConstant;
using pipelineInfo = PIPELINE::pipelineInfo;
using descriptorLayoutInfo = PIPELINE::descriptorLayoutInfo;

class DeviceHandler;

class PipelineManager
{
private:
    DeviceHandler* m_deviceHandler{nullptr};
    VkDevice m_logicalDevice;
    std::function<void(const VkResult&)> m_checkVkResult;

    // maps to store the important DATA
    std::map<std::string, pipelineInfo> m_pipelines;
    std::map<std::string, descriptorLayoutInfo> m_descriptorLayouts;
    std::map<std::string, VkPipelineLayout> m_pipelineLayout;

    std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::string& path);
    void createBaseDescriptorSetLayouts();

    void createBasePipelineLayouts();

public:

    PipelineManager() = delete;
    PipelineManager(DeviceHandler* de);
    ~PipelineManager();
    // rule of many
    PipelineManager(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager& operator=(PipelineManager&&) = delete;

    pipelineInfo &addBaseGraphicsPipelineCreateInfo(const std::string& name);
    VkPipelineLayout& createGraphicsPipeline(VkPipeline& newPipeline, const std::string& name, const std::string& pipelineLayoutName, const std::string& vertPath, const std::string& fragPath);
    void addVertexDataToPipeline(const std::string& vertexName, const std::string& pipelineName);

    void createUBODescriptorSet(VkDescriptorSet& setToCreate, VkBuffer& buffer, VkDeviceSize offset, VkDeviceSize range);
    void createSamplerDescriptorSet(VkDescriptorSet& setToCreate, VkImageView& imageView);
};

#endif
