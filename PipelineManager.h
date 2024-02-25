#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <map>
#include <string>
#include <vulkan/vulkan.h>
#include <functional>

class DeviceHandler;

class PipelineManager
{
private:
    DeviceHandler* m_deviceHandler{nullptr};
    VkDevice m_logicalDevice;
    std::function<void(const VkResult&)> m_checkVkResult;

    struct pipelineInfo
    {
        std::string vertShaderPath{""};
        std::string fragShaderPath{""};
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        std::vector<VkDynamicState> dynamicStates;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        VkPipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineLayout pipelineLayout{};
        VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        VkPipeline pipeline;
    };

    // maps to strore the important DATA
    std::map<std::string, pipelineInfo> m_pipelines;
    std::map<std::string, VkPipelineLayout> m_pipelineLayouts;

    std::vector<char> readFile(const std::string& path);
    VkShaderModule createShaderModule(const std::string& path);

public:

    PipelineManager() = delete;
    PipelineManager(DeviceHandler* de);
    ~PipelineManager();

    // can add more type of pipeline: compute for example
    PipelineManager::pipelineInfo &addBaseGraphicsPipelineCreateInfo(const std::string& name);
    PipelineManager::pipelineInfo &getGraphicsPipelineInfo(const std::string& name) { return m_pipelines[name]; };
    void createGraphicsPipeline(const std::string& name, const std::string& layoutName, const std::string& vertPath, const std::string& fragPath);
    VkPipeline &getPipeline(const std::string& name) { return m_pipelines[name].pipeline; };
    void addPipelineLayout(const std::string& name, const VkPipelineLayoutCreateInfo& createInfo);
    VkPipelineLayout &getPipelineLayout(const std::string& name) { return m_pipelineLayouts[name]; };

};

#endif
