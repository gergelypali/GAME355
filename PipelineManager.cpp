#include "PipelineManager.h"
#include "DeviceHandler.h"
#include "Logger.h"
#include <iostream>

PipelineManager::PipelineManager(DeviceHandler *de)
    : m_deviceHandler(de)
{
    m_logicalDevice = m_deviceHandler->getLogicalDevice();
    m_checkVkResult = std::bind(&DeviceHandler::checkVkResult, m_deviceHandler, std::placeholders::_1);

    std::vector<descriptorCreateInfo> newVector;
    descriptorCreateInfo new1{};
    new1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    new1.bufferSize = sizeof(uboData);
    new1.numberOfMaxSets = 5;
    new1.shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    newVector.push_back(new1);

    m_descriptorCreateInfos.insert({"baseUBO", newVector});
    initDescriptorConfig();
}

PipelineManager::~PipelineManager()
{
    for (auto& [key, value]: m_descriptorDatas)
    {
        for (auto& bufferMem: value.bufferMemory)
        { vkUnmapMemory(m_logicalDevice, bufferMem); }
        for (auto& buffer: value.buffer)
        { vkDestroyBuffer(m_logicalDevice, buffer, VK_NULL_HANDLE); }
        for (auto& bufferMem: value.bufferMemory)
        { vkFreeMemory(m_logicalDevice, bufferMem, VK_NULL_HANDLE); }
        vkDestroyDescriptorSetLayout(m_logicalDevice, value.setLayout, nullptr);
    }
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    for (auto& [key, value]: m_pipelines)
    { vkDestroyPipeline(m_logicalDevice, value.pipeline, nullptr); }
    for (auto& [key, value]: m_pipelineLayouts)
    { vkDestroyPipelineLayout(m_logicalDevice, value.layout, nullptr); }
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

void PipelineManager::createGraphicsPipeline(
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
    createInfo.layout = m_pipelineLayouts[pipelineLayoutName].layout;
    createInfo.renderPass = m_deviceHandler->getRenderPass();
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline{VK_NULL_HANDLE};

    m_checkVkResult(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &newPipeline));
    m_pipelines[name].pipeline = newPipeline;
    m_pipelines[name].pipelineInfo = createInfo;

    vkDestroyShaderModule(m_logicalDevice, vertShader, nullptr);
    vkDestroyShaderModule(m_logicalDevice, fragShader, nullptr);
    Logger::Instance()->logError("createGraphicsPipeline: 2");
}

pipelineLayoutInfo &PipelineManager::addVec4PushConstantPipelineLayout(const std::string &name)
{
    Logger::Instance()->logError("addVec4PushConstantPipelineLayout: 1");
    if (m_pipelineLayouts.find(name) == m_pipelineLayouts.end())
        m_pipelineLayouts.insert({name, pipelineLayoutInfo{}});

    m_pipelineLayouts[name].layoutCreateInfo.pushConstantRangeCount = 2;
    // GOODTOKNOW: you need separate VkPushConstantRange-s for every shader stage
    // you cannot ORd up the two shaders in one stageFlags; or use VK_SHADER_STAGE_ALL_GRAPHICS
    VkPushConstantRange pushCV;
    pushCV.size = sizeof(vec4PC);
    pushCV.offset = 0;
    pushCV.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    VkPushConstantRange pushCF;
    pushCF.size = sizeof(vec4PC);
    pushCF.offset = sizeof(vec4PC);
    pushCF.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    std::vector<VkPushConstantRange> range = {pushCV, pushCF};
    m_pipelineLayouts[name].layoutCreateInfo.pPushConstantRanges = range.data();

    Logger::Instance()->logError("addVec4PushConstantPipelineLayout: 2");
    return m_pipelineLayouts[name];
}

pipelineLayoutInfo &PipelineManager::addDescriptorSetToPipelineLayout(const std::string &descriptorSetName, const std::string &pipelineLayoutName)
{
    Logger::Instance()->logError("addDescriptorSetToPipelineLayout: 1");
    if (m_descriptorDatas.find(descriptorSetName) == m_descriptorDatas.end())
        throw std::out_of_range("PipelineManager: cannot find desciptorSet: " + descriptorSetName);
    if (m_pipelineLayouts.find(pipelineLayoutName) == m_pipelineLayouts.end())
        m_pipelineLayouts.insert({pipelineLayoutName, pipelineLayoutInfo{}});

    m_pipelineLayouts[pipelineLayoutName].layouts.push_back(m_descriptorDatas[descriptorSetName].setLayout);
    m_pipelineLayouts[pipelineLayoutName].layoutCreateInfo.setLayoutCount = (uint32_t)m_pipelineLayouts[pipelineLayoutName].layouts.size();
    m_pipelineLayouts[pipelineLayoutName].layoutCreateInfo.pSetLayouts = m_pipelineLayouts[pipelineLayoutName].layouts.data();
    Logger::Instance()->logError("addDescriptorSetToPipelineLayout: 2");

    return m_pipelineLayouts[pipelineLayoutName];
}

void PipelineManager::initDescriptorConfig()
{
    Logger::Instance()->logError("initDescriptorSet: 1");
    //TODO: if we have multiple UBOs we have to store the offsets and use them in the update function
    uint8_t uboCount{0};
    for (auto& [key, value]: m_descriptorCreateInfos)
    {
        Logger::Instance()->logError("initDescriptorSet: 2");
        descriptorData finalData{};
        finalData.buffer.resize(value.size(), VkBuffer{});
        finalData.bufferMemory.resize(value.size(), VkDeviceMemory{});
        finalData.bufferMemoryAddress.resize(value.size(), VK_NULL_HANDLE);

        // create all of the descriptors binding
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (size_t i = 0; i < value.size(); i++)
        {
            Logger::Instance()->logError("initDescriptorSet: 3");
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = value[i].type;
            layoutBinding.stageFlags = value[i].shaderStageFlags;
            bindings.push_back(layoutBinding);
            if (value[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                Logger::Instance()->logError("initDescriptorSet: 4");
                uboCount += value[i].numberOfMaxSets;
                // create the buffer memory part if we have a uniformbuffer
                m_deviceHandler->createBuffer(
                    value[i].bufferSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    finalData.buffer[i],
                    finalData.bufferMemory[i]
                    );
                m_checkVkResult(vkMapMemory(
                    m_logicalDevice,
                    finalData.bufferMemory[i],
                    0,
                    value[i].bufferSize,
                    0,
                    &finalData.bufferMemoryAddress[i]
                    ));
            }
        }
        Logger::Instance()->logError("initDescriptorSet: 5");
        VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = (uint32_t)bindings.size();
        layoutInfo.pBindings = bindings.data();

        // create the descriptorsetlayout from the bindings and store it in the map
        m_checkVkResult(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, VK_NULL_HANDLE, &finalData.setLayout));
        m_descriptorDatas.insert({key, finalData});
    }
    Logger::Instance()->logError("initDescriptorSet: 6");
    std::vector<VkDescriptorPoolSize> poolSizes;

    // ubo part
    VkDescriptorPoolSize uboPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
    uboPoolSize.descriptorCount = (uint32_t)uboCount;
    poolSizes.push_back(uboPoolSize);

    // create the pool
    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 20;//this is hardcoded here; maybe change later if needed

    m_checkVkResult(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, VK_NULL_HANDLE, &m_descriptorPool));
    Logger::Instance()->logError("initDescriptorSet: 7");
    for (auto& [key, value]: m_descriptorDatas)
    {
        Logger::Instance()->logError("initDescriptorSet: 8");
        VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &value.setLayout;

        m_checkVkResult(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &value.set));

        // update the descriptorset with the real memory objects
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        for (size_t i = 0; i < value.buffer.size(); i++)
        {
            Logger::Instance()->logError("initDescriptorSet: 9");
            VkWriteDescriptorSet descriptorWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptorWrite.dstBinding = (uint32_t)i;
            descriptorWrite.dstSet = value.set;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = m_descriptorCreateInfos[key][i].type;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = VK_NULL_HANDLE;
            descriptorWrite.pImageInfo  = VK_NULL_HANDLE;
            descriptorWrite.pTexelBufferView  = VK_NULL_HANDLE;

            if(m_descriptorCreateInfos[key][i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                Logger::Instance()->logError("initDescriptorSet: 10");
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = value.buffer[i];
                bufferInfo.offset = 0;
                bufferInfo.range = m_descriptorCreateInfos[key][i].bufferSize;
                descriptorWrite.pBufferInfo = &bufferInfo;
            }
            descriptorWrites.push_back(descriptorWrite);
        }
        vkUpdateDescriptorSets(m_logicalDevice, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, VK_NULL_HANDLE);
        Logger::Instance()->logError("initDescriptorSet: 11");
    }
    Logger::Instance()->logError("initDescriptorSet: 12");
}

void PipelineManager::createPipelineLayout(const std::string& name)
{
    Logger::Instance()->logError("createPipelineLayout: 1");
    if (m_pipelineLayouts.find(name) == m_pipelineLayouts.end())
    {
        Logger::Instance()->logCritical("PipelineManager cannot find basePipelineLayout, so we cannot create this pipelineLayout: " + name);
        return;
    }
    m_checkVkResult(vkCreatePipelineLayout(
        m_logicalDevice,
        &m_pipelineLayouts[name].layoutCreateInfo,
        VK_NULL_HANDLE,
        &m_pipelineLayouts[name].layout
        ));
    Logger::Instance()->logError("createPipelineLayout: 2");
}

void PipelineManager::updateUbo(uboData &newUbo, const std::string& name, uint32_t binding)
{
    memcpy(m_descriptorDatas[name].bufferMemoryAddress[binding], &newUbo, sizeof(newUbo));
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
