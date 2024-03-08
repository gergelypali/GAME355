#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include "Entity.h"
#include <math.h>
#include "Logger.h"
#include "Rectangle.h"

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_windowX = m_deviceHandler->getWindowSize().width / 2;
    m_windowY = m_deviceHandler->getWindowSize().height / 2;

    Rectangle* theRectangle = new Rectangle(m_deviceHandler, m_pipelineManager);
    m_renderTheseObjects.insert({"rectangle", theRectangle});

    // we need to call this before pipeline creation and after all of the renderObjects are in the map
    m_pipelineManager->initDescriptorConfig();

    for (auto& obj: m_renderTheseObjects) { obj.second->createPipeline(); }

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(1);
    createSecondaryCommandBuffer(m_secondaryCommandBuffer);
}

VulkanRenderer::~VulkanRenderer()
{
    for (auto& obj: m_renderTheseObjects) { delete obj.second; }
    m_renderTheseObjects.clear();
    delete m_pipelineManager;
    delete m_deviceHandler;
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
    for (auto& obj: m_renderTheseObjects) { obj.second->updateUBO(); }

    // TODO: we need to recreate them because we have to provide the framebuffer that is used
    // and it is changing per frame
    // if I need more optimalization I can just create this once hopefully, but now I just left this here
    // create a secondary commandbuffer for all of the rectangles we need to draw
    for (auto& obj: m_renderTheseObjects)
    {
        // this hardcoded 0 is not good if we have more objects to draw later
        obj.second->createCommandBuffer(m_secondaryCommandBuffer[0]);
    }

    // create the main command buffer with all of the secondary command buffers
    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);

    // after we update all of the UBOs we can render the frame
    m_deviceHandler->drawFrame(m_primaryCommandBuffer);

    // reset all of the variables so we can start and handle the next frame
    for (auto& obj: m_renderTheseObjects) { obj.second->resetFrameVariables(); }
}

void VulkanRenderer::vulkanRenderRect(const MATH::Vec4& positionAndSize, const MATH::Vec4& color)
{
    auto rectangle = static_cast<Rectangle*>(m_renderTheseObjects["rectangle"]);
    rectangle->m_ubodata.positionAndSize[rectangle->m_rectangleCount] = MATH::Vec4{(positionAndSize.x /m_windowX - 1), (positionAndSize.y / m_windowY - 1), positionAndSize.z/(float)m_windowX, positionAndSize.w/(float)m_windowY};
    rectangle->m_ubodata.color[rectangle->m_rectangleCount] = color;
    if(rectangle->m_rectangleCount < 1000)
        rectangle->m_rectangleCount += 1;
}
