#include "Shape2d.h"
#include "Logger.h"
#include "PipelineManager.h"
#include <vulkan/vulkan.h>

void Shape2d::init()
{

}

std::vector<descriptorCreateInfo> Shape2d::createDescriptorCreateInfo()
{
    std::vector<descriptorCreateInfo> uboCreateInfo;
    descriptorCreateInfo uboBinding0{};
    uboBinding0.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding0.bufferSize = sizeof(shape2dUboData);
    uboBinding0.numberOfMaxSets = 5;
    uboBinding0.shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboCreateInfo.push_back(uboBinding0);

    Logger::Instance()->logInfo("Shape2d: createDescriptorCreateInfo DONE");
    return uboCreateInfo;
}

void Shape2d::createPipeline(PipelineManager* pm)
{
    pm->addBaseGraphicsPipelineCreateInfo(m_name + "Pipeline");
    pm->addVertexDataToPipeline("rectangle", m_name + "Pipeline");
    pm->addDescriptorSetToPipelineLayout(m_name + "UBO", m_name + "PipelineLayout");
    pm->createPipelineLayout(m_name + "PipelineLayout");
    pm->createGraphicsPipeline(m_name + "Pipeline", m_name + "PipelineLayout", "shaders/shape2dShaderVert.spv", "shaders/shape2dShaderFrag.spv");

    Logger::Instance()->logInfo("Shape2d: createPipeline DONE");
}

void Shape2d::updateUBO(void* address)
{
    auto itPos = std::begin(m_ubodata.positionAndSize);
    auto endPos = std::end(m_ubodata.positionAndSize);
    auto itCol = std::begin(m_ubodata.color);
    auto endCol = std::end(m_ubodata.color);
    for (auto& [key, value]: m_vertexData)
    {
        for (auto& obj: value)
        {
            if (itPos == endPos || itCol == endCol)
                break;
            *itPos = obj.first;
            *itCol = obj.second;
            itPos++;
            itCol++;
        }
    }

    memcpy(address, &m_ubodata, sizeof(m_ubodata));
}

void Shape2d::resetFrameVariables()
{
    m_shapeCount = 0;// this is not used now, but maybe later so I keep it here
    m_vertexData.clear();
    m_vertexBufferMap.clear();
    m_indexBufferMap.clear();
}

void Shape2d::createCommandBuffer(VkCommandBuffer& buffer)
{
    int instanceOffset{0};
    for (auto& obj: m_vertexData)
    {
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(buffer, 0, 1, &m_vertexBufferMap[obj.first], offsets);
        vkCmdBindIndexBuffer(buffer, m_indexBufferMap[obj.first].first, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(buffer, m_indexBufferMap[obj.first].second, m_vertexData[obj.first].size(), 0, 0, instanceOffset);
        instanceOffset += m_vertexData[obj.first].size();
    }
}

void Shape2d::addShape2dToDraw(const std::string &name, MATH::Vec4 &positionAndSize, MATH::Vec4 &color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount)
{
    if (m_vertexData.find(name) == m_vertexData.end())
        m_vertexData.insert({ name, {} });

    auto insertThis = std::make_pair(positionAndSize, color);
    m_vertexData[name].push_back(insertThis);

    m_vertexBufferMap[name] = vertexBuffer;
    m_indexBufferMap[name] = std::make_pair(indexBuffer, indexCount);
}
