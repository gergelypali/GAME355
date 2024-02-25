#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include "Entity.h"
#include <math.h>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_pipelineManager->addVec4PushConstantPipelineLayout("baseLayout");
    m_pipelineManager->createPipelineLayout("baseLayout");
    m_pipelineManager->addBaseGraphicsPipelineCreateInfo("base");
    m_pipelineManager->createGraphicsPipeline("base", "baseLayout", "shaders/baseShaderVert.spv", "shaders/baseShaderFrag.spv");

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(1);
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
    /*
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0], m_pipelineManager->getPipeline("base"));
    PipelineManager::vec4PC sendIt;
    sendIt.vector = MATH::Vec4{0.1,0.1,0,0};
    vkCmdPushConstants(m_secondaryCommandBuffer[0], m_pipelineManager->getPipelineLayout("baseLayout"), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(sendIt), &sendIt);
    vkCmdDraw(m_secondaryCommandBuffer[0], 6, 1, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[1], m_pipelineManager->getPipeline("base"));
    vkCmdPushConstants(m_secondaryCommandBuffer[1], m_pipelineManager->getPipelineLayout("baseLayout"), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(sendIt), &sendIt);
    vkCmdDraw(m_secondaryCommandBuffer[1], 4, 1, 0, 1);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[1]);
    in this method we will have only a render primary commandbuffer and thats all
    */
    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    m_deviceHandler->drawFrame(m_primaryCommandBuffer);
}

void VulkanRenderer::vulkanRenderRect(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>())
        return;
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();

    uint32_t windowX = m_deviceHandler->getWindowSize().width / 2;
    uint32_t windowY = m_deviceHandler->getWindowSize().height / 2;
    PipelineManager::vec4PC sendIt;
    sendIt.vector = MATH::Vec4{(transform.pos.x - windowX)/windowX, (transform.pos.y - windowY)/windowY, 0, 0};
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0], m_pipelineManager->getPipeline("base"));
    vkCmdPushConstants(m_secondaryCommandBuffer[0], m_pipelineManager->getPipelineLayout("baseLayout"), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(sendIt), &sendIt);
    vkCmdDraw(m_secondaryCommandBuffer[0], 6, 1, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);
}
