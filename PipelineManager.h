#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <map>
#include <string>
#include <vulkan/vulkan.h>
#include <functional>
#include "Vector.h"

#include "Structs.h"

using pipelineInfo = PIPELINE::pipelineInfo;
using pipelineLayoutInfo = PIPELINE::pipelineLayoutInfo;
using vec4PC = BUFFER::vec4PushConstant;

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
    struct descriptorCreateInfo
    {
        VkDescriptorType type;
        VkShaderStageFlags shaderStageFlags;
        VkDeviceSize bufferSize;
        uint8_t numberOfMaxSets{0};
    };

    std::map<std::string, std::vector<descriptorCreateInfo>> m_descriptorCreateInfos;
    std::map<std::string, descriptorData> m_descriptorDatas;
    VkDescriptorPool m_descriptorPool;

    void initDescriptorConfig();

public:

    struct uboData
    {
        MATH::Vec4 vector[1000];
    };

    PipelineManager() = delete;
    PipelineManager(DeviceHandler* de);
    ~PipelineManager();

    // can add more type of pipeline: compute for example
    // pipeline part
    pipelineInfo &addBaseGraphicsPipelineCreateInfo(const std::string& name);
    void createGraphicsPipeline(const std::string& name, const std::string& pipelineLayoutName, const std::string& vertPath, const std::string& fragPath);
    pipelineInfo &getGraphicsPipelineInfo(const std::string& name) { return m_pipelines[name]; };
    VkPipeline &getPipeline(const std::string& name) { return m_pipelines[name].pipeline; };

    // pipelinelayout part
    pipelineLayoutInfo &addVec4PushConstantPipelineLayout(const std::string& name);
    pipelineLayoutInfo &addDescriptorSetToPipelineLayout(const std::string &descriptorSetName, const std::string &pipelineLayoutName);
    void createPipelineLayout(const std::string& name);
    VkPipelineLayout &getPipelineLayout(const std::string& name) { return m_pipelineLayouts[name].layout; };

    // descriptorSet part
    VkDescriptorSet &getDescriptorSet(const std::string& name) { return m_descriptorDatas[name].set; };
    void updateUbo(uboData &newUbo, const std::string& name, uint32_t binding);
};

#endif
