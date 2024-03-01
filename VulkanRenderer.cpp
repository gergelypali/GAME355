#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "Entity.h"
#include <math.h>
#include "Logger.h"

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_windowX = m_deviceHandler->getWindowSize().width / 2;
    m_windowY = m_deviceHandler->getWindowSize().height / 2;

    m_pipelineManager->addBaseGraphicsPipelineCreateInfo("rectanglePipeline");
    m_pipelineManager->addDescriptorSetToPipelineLayout("rectangleUBO", "rectanglePipelineLayout");
    m_pipelineManager->createPipelineLayout("rectanglePipelineLayout");
    m_pipelineManager->createGraphicsPipeline("rectanglePipeline", "rectanglePipelineLayout", "shaders/rectangleShaderVert.spv", "shaders/rectangleShaderFrag.spv");

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
    // update the UBOs to transfer the new data to the shaders
    m_pipelineManager->updateRectangleUbo(m_rectangleUbo, "rectangleUBO", 0);

    // TODO: we need to recreate them because we have to provide the framebuffer that is used
    // and it is changing per frame
    // if I need more optimalization I can just create this once hopefully, but now I just left this here
    // create a secondary commandbuffer for all of the rectangles we need to draw
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0], m_pipelineManager->getPipeline("rectanglePipeline"));
    vkCmdBindDescriptorSets(m_secondaryCommandBuffer[0], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager->getPipelineLayout("rectanglePipelineLayout"), 0, 1, &m_pipelineManager->getDescriptorSet("rectangleUBO"), 0, VK_NULL_HANDLE);
    vkCmdDraw(m_secondaryCommandBuffer[0], 6, m_rectangleCount, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);

    // create the main command buffer with all of the secondary command buffers
    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    // after we upadte all of the UBOs we can render the frame
    m_deviceHandler->drawFrame(m_primaryCommandBuffer);

    // reset all of the variables so we can start and handle the next frame
    m_rectangleCount = 0;
}

void VulkanRenderer::vulkanRenderRect(const MATH::Vec4& positionAndSize, const MATH::Vec4& color)
{
    m_rectangleUbo.positionAndSize[m_rectangleCount] = MATH::Vec4{(positionAndSize.x /m_windowX - 1), (positionAndSize.y / m_windowY - 1), positionAndSize.z/(float)m_windowX, positionAndSize.w/(float)m_windowY};
    m_rectangleUbo.color[m_rectangleCount] = color;
    if(m_rectangleCount < 1000)
        m_rectangleCount += 1;
}
