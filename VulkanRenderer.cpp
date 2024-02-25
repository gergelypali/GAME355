#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include <math.h>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    // dirty hack to have a createInfo here and I can create a layout fast
    VkPipelineLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.setLayoutCount = 0;
    createInfo.pushConstantRangeCount = 1;
    //pushconstant
    VkPushConstantRange pushC;
    pushC.size = sizeof(pushConstant);
    pushC.offset = 0;
    pushC.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.pPushConstantRanges = &pushC;
    // hack end
    m_pipelineManager->addPipelineLayout("baseLayout", createInfo);
    m_pipelineManager->addBaseGraphicsPipelineCreateInfo("base");
    m_pipelineManager->createGraphicsPipeline("base", "baseLayout", "shaders/baseShaderVert.spv", "shaders/baseShaderFrag.spv");

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(2);
    createSecondaryCommandBuffer(m_secondaryCommandBuffer);
}

VulkanRenderer::~VulkanRenderer()
{
    delete(m_pipelineManager);
    delete(m_deviceHandler);
}

void VulkanRenderer::createPrimaryCommandBuffer(VkCommandBuffer& buffer)
{
    m_deviceHandler->createCommandBuffer(buffer, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void VulkanRenderer::createSecondaryCommandBuffer(std::vector<VkCommandBuffer>& buffer)
{
    m_deviceHandler->createCommandBuffer(buffer, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY);
}

void VulkanRenderer::drawFrame()
{
    static int lol{0};
    MATH::Vec4 position{sinf(lol*0.05f) / 2, cosf(lol*0.1f) / 2, 0, 0};
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0], m_pipelineManager->getPipeline("base"));
    m_deviceHandler->sendPushConstant(m_secondaryCommandBuffer[0], position, m_pipelineManager->getPipelineLayout("baseLayout"));
    vkCmdDraw(m_secondaryCommandBuffer[0], 4, 1, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[1], m_pipelineManager->getPipeline("base"));
    m_deviceHandler->sendPushConstant(m_secondaryCommandBuffer[1], position, m_pipelineManager->getPipelineLayout("baseLayout"));
    vkCmdDraw(m_secondaryCommandBuffer[1], 4, 1, 0, 1);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[1]);

    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    m_deviceHandler->drawFrame(m_primaryCommandBuffer);
    lol += 1;
}

void VulkanRenderer::vulkanRenderRect()
{

}
