#include "Rectangle.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include "Logger.h"

void Rectangle::init()
{

}

std::vector<descriptorCreateInfo> Rectangle::createDescriptorCreateInfo()
{
    // descriptor for the ubo for rectangle rendering
    std::vector<descriptorCreateInfo> uboCreateInfo;
    descriptorCreateInfo uboBinding0{};
    uboBinding0.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding0.bufferSize = sizeof(rectangleUboData);
    uboBinding0.numberOfMaxSets = 5;
    uboBinding0.shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboCreateInfo.push_back(uboBinding0);

    Logger::Instance()->logInfo("Rectangle: createDescriptorCreateInfo DONE");
    return uboCreateInfo;
}

void Rectangle::createPipeline(PipelineManager* pm)
{
    pm->addBaseGraphicsPipelineCreateInfo(m_name + "Pipeline");
    //pm->addVertexDataToPipeline(m_name, m_name + "Pipeline");
    pm->addDescriptorSetToPipelineLayout(m_name + "UBO", m_name + "PipelineLayout");
    pm->createPipelineLayout(m_name + "PipelineLayout");
    pm->createGraphicsPipeline(m_name + "Pipeline", m_name + "PipelineLayout", "shaders/rectangleShaderVert.spv", "shaders/rectangleShaderFrag.spv");

    Logger::Instance()->logInfo("Rectangle: createPipeline DONE");
}

void Rectangle::updateUBO(void* address)
{
    memcpy(address, &m_ubodata, sizeof(m_ubodata));
}

void Rectangle::resetFrameVariables()
{
    m_rectangleCount = 0;
    // we have to clear the buffer also, so when we delete an object from the screen it will disappear also; no, that is why we have this counter, so only this many objects will be rendered to the screen
}

void Rectangle::createCommandBuffer(VkCommandBuffer& buffer)
{
    vkCmdDraw(buffer, 6, m_rectangleCount, 0, 0);
}
