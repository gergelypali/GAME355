#include "PipelineManager.h"
#include "DeviceHandler.h"
#include "Logger.h"
#include <iostream>

PipelineManager::PipelineManager(DeviceHandler *de)
    : m_deviceHandler(de)
{
    m_logicalDevice = m_deviceHandler->getLogicalDevice();
    m_checkVkResult = std::bind(&DeviceHandler::checkVkResult, m_deviceHandler, std::placeholders::_1);

    createBaseDescriptorSetLayouts();
    createBasePipelineLayouts();
}

PipelineManager::~PipelineManager()
{
    for (auto& [key, value]: m_pipelines)
    { vkDestroyPipeline(m_logicalDevice, value.pipeline, nullptr); }
    for (auto& [key, value]: m_pipelineLayout)
    { vkDestroyPipelineLayout(m_logicalDevice, value, nullptr); }
    for (auto& [key, value]: m_descriptorLayouts)
    {
        vkDestroyDescriptorPool(m_logicalDevice, value.allocInfo.descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_logicalDevice, value.layout, VK_NULL_HANDLE);
    }
}

void PipelineManager::addVertexDataToPipeline(const std::string& vertexName, const std::string& pipelineName)
{
    Logger::Instance()->logVerbose("addVertexDataToPipeline 1");
    VERTEX::baseVertex* vertexData{nullptr};
    Logger::Instance()->logVerbose("addVertexDataToPipeline 2");
    if (vertexName == "rectangle")
    {
        vertexData = new VERTEX::baseRectangle();
        Logger::Instance()->logVerbose("addVertexDataToPipeline 3");
    }
    else if (vertexName == "shape2d")
    {
        vertexData = new VERTEX::shape2dwithtexture();
        Logger::Instance()->logVerbose("addVertexDataToPipeline 3.5");
    }
    else
    {
        Logger::Instance()->logError("PipelineManager: cannot find VertexData to add named: " + vertexName);
        return;
    }
    Logger::Instance()->logVerbose("addVertexDataToPipeline 4");
    m_pipelines[pipelineName].vertexInputBindings = vertexData->getBindingDescriptor();
    Logger::Instance()->logVerbose("addVertexDataToPipeline 5");
    m_pipelines[pipelineName].vertexInputAttributes = vertexData->getAttributeDescriptor();
    Logger::Instance()->logVerbose("addVertexDataToPipeline 6");
    m_pipelines[pipelineName].vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)m_pipelines[pipelineName].vertexInputAttributes.size();
    m_pipelines[pipelineName].vertexInputCreateInfo.pVertexAttributeDescriptions = m_pipelines[pipelineName].vertexInputAttributes.data();
    m_pipelines[pipelineName].vertexInputCreateInfo.vertexBindingDescriptionCount = (uint32_t)m_pipelines[pipelineName].vertexInputBindings.size();
    m_pipelines[pipelineName].vertexInputCreateInfo.pVertexBindingDescriptions = m_pipelines[pipelineName].vertexInputBindings.data();
    Logger::Instance()->logVerbose("addVertexDataToPipeline 7");
    delete vertexData;
}

pipelineInfo &PipelineManager::addBaseGraphicsPipelineCreateInfo(const std::string &name)
{
    Logger::Instance()->logError("addBaseGraphicsPipelineCreateInfo: 1");
    if (m_pipelines.find(name) == m_pipelines.end())
        m_pipelines.insert({name, pipelineInfo{}});

    //VkPipelineVertexInputStateCreateInfo
    m_pipelines[name].vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    m_pipelines[name].vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    m_pipelines[name].vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    m_pipelines[name].vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    //VkPipelineInputAssemblyStateCreateInfo
    m_pipelines[name].inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

    Logger::Instance()->logError("addBaseGraphicsPipelineCreateInfo: 2");
    return m_pipelines[name];
}

VkPipelineLayout& PipelineManager::createGraphicsPipeline(
    VkPipeline& newPipeline,
    const std::string& name,
    const std::string& pipelineLayoutName,
    const std::string& vertPath,
    const std::string& fragPath
    )
{
    Logger::Instance()->logError("createGraphicsPipeline: 1");
    if (m_pipelines.find(name) == m_pipelines.end())
    {
        Logger::Instance()->logCritical("PipelineManager cannot find basegraphicspipeline, so we cannot create this pipeline: " + name);
        throw;
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
    createInfo.layout = m_pipelineLayout[pipelineLayoutName];
    createInfo.renderPass = m_deviceHandler->getRenderPass();
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

    m_checkVkResult(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &newPipeline));
    m_pipelines[name].pipeline = newPipeline;
    m_pipelines[name].pipelineInfo = createInfo;

    vkDestroyShaderModule(m_logicalDevice, vertShader, nullptr);
    vkDestroyShaderModule(m_logicalDevice, fragShader, nullptr);
    Logger::Instance()->logError("createGraphicsPipeline: 2");

    return m_pipelineLayout[pipelineLayoutName];
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

// this is the new code for a more hardcoded pipeline and descriptorset creation
void PipelineManager::createBaseDescriptorSetLayouts()
{
    //ubo part
    m_descriptorLayouts.insert({"ubo0vertex", descriptorLayoutInfo{}});

    VkDescriptorSetLayoutBinding ubo0VertexBinding{};
    ubo0VertexBinding.binding = 0;
    ubo0VertexBinding.descriptorCount = 1;
    ubo0VertexBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo0VertexBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo ubo0vertexCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    ubo0vertexCreateInfo.bindingCount = 1;
    ubo0vertexCreateInfo.pBindings = &ubo0VertexBinding;

    m_checkVkResult(vkCreateDescriptorSetLayout(m_logicalDevice, &ubo0vertexCreateInfo, VK_NULL_HANDLE, &m_descriptorLayouts["ubo0vertex"].layout));

    VkDescriptorPoolSize uboPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
    uboPoolSize.descriptorCount = 1;
    VkDescriptorPoolCreateInfo ubopoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ubopoolCreateInfo.poolSizeCount = 1;
    ubopoolCreateInfo.pPoolSizes = &uboPoolSize;
    ubopoolCreateInfo.maxSets = 5U;//this is hardcoded here; maybe change later if needed

    VkDescriptorPool uboPool;
    m_checkVkResult(vkCreateDescriptorPool(m_logicalDevice, &ubopoolCreateInfo, VK_NULL_HANDLE, &uboPool));

    m_descriptorLayouts["ubo0vertex"].allocInfo.descriptorPool = uboPool;
    m_descriptorLayouts["ubo0vertex"].allocInfo.descriptorSetCount = 1;
    m_descriptorLayouts["ubo0vertex"].allocInfo.pSetLayouts = &m_descriptorLayouts["ubo0vertex"].layout;

    //combined sampler part
    m_descriptorLayouts.insert({"sampler1fragment", descriptorLayoutInfo{}});
    
    VkDescriptorSetLayoutBinding sampler1FragmentBinding{};
    sampler1FragmentBinding.binding = 1;
    sampler1FragmentBinding.descriptorCount = 1;
    sampler1FragmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler1FragmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo sampler1fragmentCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    sampler1fragmentCreateInfo.bindingCount = 1;
    sampler1fragmentCreateInfo.pBindings = &sampler1FragmentBinding;

    m_checkVkResult(vkCreateDescriptorSetLayout(m_logicalDevice, &sampler1fragmentCreateInfo, VK_NULL_HANDLE, &m_descriptorLayouts["sampler1fragment"].layout));

    VkDescriptorPoolSize samplerPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
    samplerPoolSize.descriptorCount = 1;
    VkDescriptorPoolCreateInfo samplerpoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    samplerpoolCreateInfo.poolSizeCount = 1;
    samplerpoolCreateInfo.pPoolSizes = &samplerPoolSize;
    samplerpoolCreateInfo.maxSets = 50U;//this is hardcoded here; maybe change later if needed

    VkDescriptorPool samplerPool;
    m_checkVkResult(vkCreateDescriptorPool(m_logicalDevice, &samplerpoolCreateInfo, VK_NULL_HANDLE, &samplerPool));

    m_descriptorLayouts["sampler1fragment"].allocInfo.descriptorPool = samplerPool;
    m_descriptorLayouts["sampler1fragment"].allocInfo.descriptorSetCount = 1;
    m_descriptorLayouts["sampler1fragment"].allocInfo.pSetLayouts = &m_descriptorLayouts["sampler1fragment"].layout;
}

void PipelineManager::createUBODescriptorSet(VkDescriptorSet& setToCreate, VkBuffer& buffer, VkDeviceSize offset, VkDeviceSize range)
{
    m_checkVkResult(vkAllocateDescriptorSets(m_logicalDevice, &m_descriptorLayouts["ubo0vertex"].allocInfo, &setToCreate));

    VkWriteDescriptorSet descriptorWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstSet = setToCreate;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo  = VK_NULL_HANDLE;
    descriptorWrite.pTexelBufferView  = VK_NULL_HANDLE;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, 1, &descriptorWrite, 0, VK_NULL_HANDLE);
}

void PipelineManager::createSamplerDescriptorSet(VkDescriptorSet& setToCreate, VkImageView& imageView)
{
    m_checkVkResult(vkAllocateDescriptorSets(m_logicalDevice, &m_descriptorLayouts["sampler1fragment"].allocInfo, &setToCreate));

    VkWriteDescriptorSet descriptorWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstSet = setToCreate;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = VK_NULL_HANDLE;
    descriptorWrite.pTexelBufferView  = VK_NULL_HANDLE;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_deviceHandler->getSampler();
    imageInfo.imageView = imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    descriptorWrite.pImageInfo  = &imageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, 1, &descriptorWrite, 0, VK_NULL_HANDLE);
}

void PipelineManager::createBasePipelineLayouts()
{
    VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(m_descriptorLayouts["ubo0vertex"].layout);
    layoutCreateInfo.setLayoutCount = (uint32_t)layouts.size();
    layoutCreateInfo.pSetLayouts = layouts.data();

    VkPipelineLayout uboLayout{};
    m_checkVkResult(vkCreatePipelineLayout(
    m_logicalDevice,
    &layoutCreateInfo,
    VK_NULL_HANDLE,
    &uboLayout
    ));

    m_pipelineLayout.insert({"ubo0", uboLayout});

    layouts.push_back(m_descriptorLayouts["sampler1fragment"].layout);
    layoutCreateInfo.setLayoutCount = (uint32_t)layouts.size();
    layoutCreateInfo.pSetLayouts = layouts.data();

    VkPipelineLayout ubosamplerLayout{};
    m_checkVkResult(vkCreatePipelineLayout(
    m_logicalDevice,
    &layoutCreateInfo,
    VK_NULL_HANDLE,
    &ubosamplerLayout
    ));

    m_pipelineLayout.insert({"ubo0sampler1", ubosamplerLayout});
}
