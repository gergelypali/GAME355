#include "PipelineManager.h"
#include "DeviceHandler.h"
#include "Logger.h"
#include <iostream>

PipelineManager::PipelineManager(DeviceHandler *de)
{
    m_deviceHandler = de;
    m_logicalDevice = m_deviceHandler->getLogicalDevice();
    m_checkVkResult = std::bind(&DeviceHandler::checkVkResult, m_deviceHandler, std::placeholders::_1);
}

PipelineManager::~PipelineManager()
{
    for (auto& [key, value]: m_pipelines)
    { vkDestroyPipeline(m_logicalDevice, value.pipeline, nullptr); }
    for (auto& [key, value]: m_pipelineLayouts)
    { vkDestroyPipelineLayout(m_logicalDevice, value, nullptr); }
}

PipelineManager::pipelineInfo &PipelineManager::addBaseGraphicsPipelineCreateInfo(const std::string &name)
{
    m_pipelines.insert({name, pipelineInfo{}});

    //VkPipelineVertexInputStateCreateInfo
    m_pipelines[name].vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    m_pipelines[name].vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    m_pipelines[name].vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    m_pipelines[name].vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    //VkPipelineInputAssemblyStateCreateInfo
    m_pipelines[name].inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    /*
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
    */
    m_pipelines[name].inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    //VkPipelineDynamicStateCreateInfo
    m_pipelines[name].dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);// <- can use these later
    m_pipelines[name].dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    m_pipelines[name].dynamicStateCreateInfo.dynamicStateCount =(uint32_t)m_pipelines[name].dynamicStates.size();
    m_pipelines[name].dynamicStateCreateInfo.pDynamicStates = m_pipelines[name].dynamicStates.data();

    //VkPipelineViewportStateCreateInfo
    m_pipelines[name].viewportStateCreateInfo.viewportCount = 1;
    m_pipelines[name].viewportStateCreateInfo.scissorCount = 1;

    //VkPipelineRasterizationStateCreateInfo
    m_pipelines[name].rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    m_pipelines[name].rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    m_pipelines[name].rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    /*
    VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
    VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
    VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
    */
    m_pipelines[name].rasterizerCreateInfo.lineWidth = 1.0f;
    m_pipelines[name].rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    m_pipelines[name].rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//this can change how I define the vertex coordinates in the vert shader
    m_pipelines[name].rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

    //VkPipelineMultisampleStateCreateInfo
    m_pipelines[name].multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    m_pipelines[name].multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // we dont need this now
    /*
    //VkPipelineDepthStencilStateCreateInfo
    m_pipelines[name].depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    m_pipelines[name].depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    m_pipelines[name].depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    m_pipelines[name].depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    m_pipelines[name].depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    */

    //VkPipelineColorBlendAttachmentState
    m_pipelines[name].colorBlendAttachmentCreateInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    m_pipelines[name].colorBlendAttachmentCreateInfo.blendEnable = VK_FALSE;

    //VkPipelineColorBlendStateCreateInfo
    m_pipelines[name].colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_pipelines[name].colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
    m_pipelines[name].colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    m_pipelines[name].colorBlendingCreateInfo.attachmentCount = 1;
    m_pipelines[name].colorBlendingCreateInfo.pAttachments = &m_pipelines[name].colorBlendAttachmentCreateInfo;
    m_pipelines[name].colorBlendingCreateInfo.blendConstants[0] = 0.0f;
    m_pipelines[name].colorBlendingCreateInfo.blendConstants[1] = 0.0f;
    m_pipelines[name].colorBlendingCreateInfo.blendConstants[2] = 0.0f;
    m_pipelines[name].colorBlendingCreateInfo.blendConstants[3] = 0.0f;

    return m_pipelines[name];
}

void PipelineManager::createGraphicsPipeline(
    const std::string& name,
    const std::string& layoutName,
    const std::string& vertPath,
    const std::string& fragPath
    )
{
    if (m_pipelines.find(name) == m_pipelines.end())
    {
        Logger::Instance()->logCritical("PipelineManager cannot find basegraphicspipeline, so we cannot create this pipeline: " + name);
        return;
    }

    VkShaderModule vertShader = createShaderModule(vertPath);
    m_pipelines[name].vertShaderPath = vertPath;
    VkShaderModule fragShader = createShaderModule(fragPath);
    m_pipelines[name].fragShaderPath = fragPath;

    VkPipelineShaderStageCreateInfo vertShaderCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderCreateInfo.module = vertShader;
    vertShaderCreateInfo.pName = "main";
    m_pipelines[name].shaderStages.push_back(vertShaderCreateInfo);

    VkPipelineShaderStageCreateInfo fragShaderCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderCreateInfo.module = fragShader;
    fragShaderCreateInfo.pName = "main";
    m_pipelines[name].shaderStages.push_back(fragShaderCreateInfo);

    VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    createInfo.pVertexInputState = &m_pipelines[name].vertexInputCreateInfo;
    createInfo.pInputAssemblyState = &m_pipelines[name].inputAssemblyCreateInfo;
    createInfo.pViewportState = &m_pipelines[name].viewportStateCreateInfo;
    createInfo.pRasterizationState = &m_pipelines[name].rasterizerCreateInfo;
    createInfo.pMultisampleState = &m_pipelines[name].multisamplingCreateInfo;
    //createInfo.pDepthStencilState = &m_pipelines[name].depthStencilCreateInfo;//dont need this right now
    createInfo.pColorBlendState = &m_pipelines[name].colorBlendingCreateInfo;
    createInfo.pDynamicState = &m_pipelines[name].dynamicStateCreateInfo;
    createInfo.stageCount = (uint32_t)m_pipelines[name].shaderStages.size();
    createInfo.pStages = m_pipelines[name].shaderStages.data();
    createInfo.layout = m_pipelineLayouts[layoutName];
    createInfo.renderPass = m_deviceHandler->getRenderPass();
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline{VK_NULL_HANDLE};

    m_checkVkResult(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &newPipeline));
    m_pipelines[name].pipeline = newPipeline;
    m_pipelines[name].pipelineInfo = createInfo;

    vkDestroyShaderModule(m_logicalDevice, vertShader, nullptr);
    vkDestroyShaderModule(m_logicalDevice, fragShader, nullptr);
}

void PipelineManager::addPipelineLayout(const std::string& name, const VkPipelineLayoutCreateInfo& createInfo)
{
    VkPipelineLayout newPipelineLayout;
    m_checkVkResult(vkCreatePipelineLayout(m_logicalDevice, &createInfo, nullptr, &newPipelineLayout));
    m_pipelineLayouts[name] = newPipelineLayout;
}

std::vector<char> PipelineManager::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        Logger::Instance()->logCritical("PipelineManager cannot open shaderFile: " + path);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule PipelineManager::createShaderModule(const std::string& path)
{
    auto shaderCode = readFile(path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule res;
    m_checkVkResult(vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &res));

    return res;
}
