#include "Shape2d.h"
#include "Logger.h"
#include "PipelineManager.h"
#include "DeviceHandler.h"
#include <vulkan/vulkan.h>
#include <cstring>//somehow we need this for macos; but not in rectangle.cpp, strange

void Shape2d::init()
{

}

void Shape2d::createPipeline(PipelineManager* pm)
{
    pm->addBaseGraphicsPipelineCreateInfo(m_name + "Pipeline");
    pm->addVertexDataToPipeline("rectangle", m_name + "Pipeline");
    Logger::Instance()->logInfo("Shape2d: createPipeline 1");
    ubo0Pipelinelayout = pm->createGraphicsPipeline(ubo0Pipeline, m_name + "Pipeline", "ubo0", "shaders/shape2dShaderVert.spv", "shaders/shape2dShaderFrag.spv");
    Logger::Instance()->logInfo("Shape2d: createPipeline 2");

    pm->addBaseGraphicsPipelineCreateInfo(m_name + "Pipeline2");
    pm->addVertexDataToPipeline("shape2d", m_name + "Pipeline2");
    ubo0Sampler1Pipelinelayout = pm->createGraphicsPipeline(ubo0Sampler1Pipeline, m_name + "Pipeline2", "ubo0sampler1", "shaders/shape2dTextureShaderVert.spv", "shaders/shape2dTextureShaderFrag.spv");
    Logger::Instance()->logInfo("Shape2d: createPipeline 3");

    pm->createUBODescriptorSet(ubo0Set, uboBuffer, 0, sizeof(m_ubodata));

    Logger::Instance()->logInfo("Shape2d: createPipeline DONE");
}

void Shape2d::createUboBuffer(DeviceHandler* dh)
{
    dh->createBuffer(
        sizeof(m_ubodata),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uboBuffer,
        uboMemory
    );

    vkMapMemory(dh->getLogicalDevice(), uboMemory, 0, sizeof(m_ubodata), 0, &uboAddress);
}

void Shape2d::updateUBO()
{
    auto itPos = std::begin(m_ubodata.positionAndSize);
    auto endPos = std::end(m_ubodata.positionAndSize);
    auto itCol = std::begin(m_ubodata.color);
    auto endCol = std::end(m_ubodata.color);
    for (auto& [key, value]: m_vertexData)
    {
        for (auto& obj: value.uboData)
        {
            if (itPos == endPos || itCol == endCol)
                break;
            *itPos = obj.first;
            *itCol = obj.second;
            itPos++;
            itCol++;
        }
    }

    memcpy(uboAddress, &m_ubodata, sizeof(m_ubodata));
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
        if (obj.second.nameTexture != "")
        {
            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ubo0Sampler1Pipeline);
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ubo0Sampler1Pipelinelayout, 0, 1, &ubo0Set, 0, VK_NULL_HANDLE);
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ubo0Sampler1Pipelinelayout, 1, 1, m_textureMap[obj.second.nameTexture], 0, VK_NULL_HANDLE);
        }
        else
        {
            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ubo0Pipeline);
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ubo0Pipelinelayout, 0, 1, &ubo0Set, 0, VK_NULL_HANDLE);
        }
        vkCmdBindVertexBuffers(buffer, 0, 1, m_vertexBufferMap[obj.second.nameVertex], offsets);
        vkCmdBindIndexBuffer(buffer, *m_indexBufferMap[obj.second.nameIndex].first, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(buffer, m_indexBufferMap[obj.second.nameIndex].second, obj.second.uboData.size(), 0, 0, instanceOffset);
        instanceOffset += obj.second.uboData.size();
    }
}

void Shape2d::addShape2dToDraw(const std::string &nameVertex, const std::string &nameIndex, MATH::Vec4 &positionAndSize, MATH::Vec4 &color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount)
{
    if (m_vertexData.find(nameVertex + nameIndex) == m_vertexData.end())
    {
        m_vertexData.insert({ nameVertex + nameIndex, {} });
        m_vertexData[nameVertex + nameIndex].nameIndex = nameIndex;
        m_vertexData[nameVertex + nameIndex].nameVertex = nameVertex;
    }

    m_vertexData[nameVertex + nameIndex].uboData.push_back(std::make_pair(positionAndSize, color));

    m_vertexBufferMap[nameVertex] = &vertexBuffer;
    m_indexBufferMap[nameIndex] = std::make_pair(&indexBuffer, indexCount);
}

void Shape2d::addShape2dToDraw(const std::string &nameVertex, const std::string &nameIndex, const std::string &nameTexture, MATH::Vec4 &positionAndSize, VkDescriptorSet& set, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount)
{
    if (m_vertexData.find(nameVertex + nameIndex + nameTexture) == m_vertexData.end())
    {
        m_vertexData.insert({ nameVertex + nameIndex + nameTexture, {} });
        m_vertexData[nameVertex + nameIndex + nameTexture].nameIndex = nameIndex;
        m_vertexData[nameVertex + nameIndex + nameTexture].nameVertex = nameVertex;
        m_vertexData[nameVertex + nameIndex + nameTexture].nameTexture = nameTexture;
    }

    m_vertexData[nameVertex + nameIndex + nameTexture].uboData.push_back(std::make_pair(positionAndSize, MATH::Vec4{0,0,0,1}));

    m_vertexBufferMap[nameVertex] = &vertexBuffer;
    m_indexBufferMap[nameIndex] = std::make_pair(&indexBuffer, indexCount);
    m_textureMap[nameTexture] = &set;
}
