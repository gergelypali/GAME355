#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include <math.h>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(2);
    createSecondaryCommandBuffer(m_secondaryCommandBuffer);
}

VulkanRenderer::~VulkanRenderer()
{
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
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[0]);
    m_deviceHandler->sendPushConstant(m_secondaryCommandBuffer[0], position);
    vkCmdDraw(m_secondaryCommandBuffer[0], 4, 1, 0, 0);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[0]);
    m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[1]);
    m_deviceHandler->sendPushConstant(m_secondaryCommandBuffer[1], position);
    vkCmdDraw(m_secondaryCommandBuffer[1], 4, 1, 0, 1);
    m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[1]);

    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    m_deviceHandler->drawFrame(m_primaryCommandBuffer);
    lol += 1;
}

void VulkanRenderer::vulkanRenderRect()
{

}
