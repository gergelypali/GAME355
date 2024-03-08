#include "Rectangle.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include "Logger.h"

void Rectangle::init()
{
    createDescriptorCreateInfo();
}

void Rectangle::createDescriptorCreateInfo()
{
    // descriptor for the ubo for rectangle rendering
    std::vector<descriptorCreateInfo> rectangleUboCreateInfo;
    descriptorCreateInfo uboBinding0{};
    uboBinding0.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding0.bufferSize = sizeof(rectangleUboData);
    uboBinding0.numberOfMaxSets = 5;
    uboBinding0.shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    rectangleUboCreateInfo.push_back(uboBinding0);

    m_pm->updateDescriptorCreateInfos(m_name + "UBO", rectangleUboCreateInfo);
    Logger::Instance()->logInfo("Rectangle: createDescriptorCreateInfo DONE");
}

void Rectangle::createPipeline()
{
    auto rectangePipeline = m_pm->addBaseGraphicsPipelineCreateInfo(m_name + "Pipeline");
    m_pm->addVertexDataToPipeline(m_name, rectangePipeline);
    m_pm->addDescriptorSetToPipelineLayout(m_name + "UBO", m_name + "PipelineLayout");
    m_pm->createPipelineLayout(m_name + "PipelineLayout");
    m_pm->createGraphicsPipeline(m_name + "Pipeline", m_name + "PipelineLayout", "shaders/rectangleShaderVert.spv", "shaders/rectangleShaderFrag.spv");

    Logger::Instance()->logInfo("Rectangle: createPipeline DONE");
}

void Rectangle::updateUBO()
{
    memcpy(m_pm->getUBOAddress("rectangleUBO")[0], &m_ubodata, sizeof(m_ubodata));
}

void Rectangle::resetFrameVariables()
{
    m_rectangleCount = 0;
}

void Rectangle::createCommandBuffer(VkCommandBuffer& buffer)
{
    m_dh->recordSecondaryCommandBufferStart(buffer, m_pm->getPipeline(m_name + "Pipeline"));
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pm->getPipelineLayout(m_name + "PipelineLayout"), 0, 1, &m_pm->getDescriptorSet(m_name + "UBO"), 0, VK_NULL_HANDLE);
    vkCmdDraw(buffer, 6, m_rectangleCount, 0, 0);
    m_dh->recordSecondaryCommandBufferEnd(buffer);

}
