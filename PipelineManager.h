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
using pipelineLayoutInfo = PIPELINE::pipelineLayoutInfo;
using descriptorCreateInfo = PIPELINE::descriptorCreateInfo;
using rectangleUboData = BUFFER::rectangleUboData;

class DeviceHandler;

class PipelineManager
{
private:
    DeviceHandler* m_deviceHandler{nullptr};
    VkDevice m_logicalDevice;
    std::function<void(const VkResult&)> m_checkVkResult;

    // maps to store the important DATA
    std::map<std::string, pipelineInfo> m_pipelines;
    std::map<std::string, pipelineLayoutInfo> m_pipelineLayouts;

    std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::string& path);

    struct descriptorData
    {
        VkDescriptorSetLayout setLayout;
        VkDescriptorSet set;
        std::vector<VkBuffer> buffer;
        std::vector<VkDeviceMemory> bufferMemory;
        std::vector<void*> bufferMemoryAddress;
    };

    std::map<std::string, std::vector<descriptorCreateInfo>> m_descriptorCreateInfos;
    std::map<std::string, descriptorData> m_descriptorDatas;
    VkDescriptorPool m_descriptorPool;

public:

    PipelineManager() = delete;
    PipelineManager(DeviceHandler* de);
    ~PipelineManager();
    // rule of many
    PipelineManager(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager& operator=(PipelineManager&&) = delete;

    // can add more type of pipeline: compute for example
    // pipeline part
    pipelineInfo &addBaseGraphicsPipelineCreateInfo(const std::string& name);
    void createGraphicsPipeline(const std::string& name, const std::string& pipelineLayoutName, const std::string& vertPath, const std::string& fragPath);
    pipelineInfo &getGraphicsPipelineInfo(const std::string& name) { return m_pipelines[name]; };
    VkPipeline &getPipeline(const std::string& name) { return m_pipelines[name].pipeline; };
    void addVertexDataToPipeline(const std::string& vertexName, const std::string& pipelineName);

    // pipelinelayout part
    pipelineLayoutInfo &addVec4PushConstantPipelineLayout(const std::string& name);
    pipelineLayoutInfo &addDescriptorSetToPipelineLayout(const std::string &descriptorSetName, const std::string &pipelineLayoutName);
    void createPipelineLayout(const std::string& name);
    VkPipelineLayout &getPipelineLayout(const std::string& name) { return m_pipelineLayouts[name].layout; };

    // descriptorSet part
    VkDescriptorSet &getDescriptorSet(const std::string& name) { return m_descriptorDatas[name].set; };
    void updateUbo(rectangleUboData &newUbo, const std::string& descriptorName, uint32_t binding);

    void updateDescriptorCreateInfos(const std::string& name, const std::vector<descriptorCreateInfo>& newCreateInfos);
    void initDescriptorConfig();
    std::vector<void*> getUBOAddress(const std::string& name) { return m_descriptorDatas[name].bufferMemoryAddress; };
};

#endif
