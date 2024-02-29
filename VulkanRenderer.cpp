#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "Entity.h"
#include <math.h>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_windowX = m_deviceHandler->getWindowSize().width / 2;
    m_windowY = m_deviceHandler->getWindowSize().height / 2;

    m_pipelineManager->addBaseGraphicsPipelineCreateInfo("base");
    //m_pipelineManager->addVec4PushConstantPipelineLayout("baseLayout");
    m_pipelineManager->addDescriptorSetToPipelineLayout("baseUBO", "baseLayout");
    m_pipelineManager->createPipelineLayout("baseLayout");
    m_pipelineManager->createGraphicsPipeline("base", "baseLayout", "shaders/baseShaderVert.spv", "shaders/baseShaderFrag.spv");

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(1);
    createSecondaryCommandBuffer(m_secondaryCommandBuffer);
}

VulkanRenderer::~VulkanRenderer()
{
    vkUnmapMemory(m_deviceHandler->getLogicalDevice(), m_largeUboMemory);
    vkDestroyBuffer(m_deviceHandler->getLogicalDevice(), m_largeUboBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_deviceHandler->getLogicalDevice(), m_largeUboMemory, VK_NULL_HANDLE);
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
    // create a secondary commandbuffer for all of the rectangles we need to draw
    m_pipelineManager->updateUbo(m_rectangleUbo, "baseUBO", 0);
    vkResetCommandBuffer(m_secondaryCommandBuffer[0], 0);
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0], m_pipelineManager->getPipeline("base"));
    vkCmdBindDescriptorSets(m_secondaryCommandBuffer[0], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager->getPipelineLayout("baseLayout"), 0, 1, &m_pipelineManager->getDescriptorSet("baseUBO"), 0, VK_NULL_HANDLE);
    vkCmdDraw(m_secondaryCommandBuffer[0], 6, m_rectangleCount, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);

    // create the main command buffer with all of the secondary command buffers
    vkResetCommandBuffer(m_primaryCommandBuffer, 0);
    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    m_deviceHandler->drawFrame(m_primaryCommandBuffer);

    // reset the counter so we will draw the right amount of rectangle
    m_rectangleCount = 0;
}

void VulkanRenderer::vulkanRenderRect(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>())
        return;
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();

    auto vector = MATH::Vec4{(transform.pos.x /m_windowX - 1), (transform.pos.y / m_windowY - 1), 0, 0};
    m_rectangleUbo.vector[m_rectangleCount] = vector;
    m_rectangleCount += 1;
}
