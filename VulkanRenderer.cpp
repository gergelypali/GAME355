#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include <math.h>
#include "Logger.h"
#include "Rectangle.h"
#include "Shape2d.h"

#include <fstream>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_windowX = m_deviceHandler->getWindowSize().width / 2;
    m_windowY = m_deviceHandler->getWindowSize().height / 2;

    //Rectangle* rectangle = new Rectangle();
    //m_renderTheseObjects.insert({"rectangle", rectangle});

    Shape2d* shape2d = new Shape2d();
    m_renderTheseObjects.insert({"shape2d", shape2d});

    for (auto& obj: m_renderTheseObjects)
    {
        auto res = obj.second->createDescriptorCreateInfo();
        m_pipelineManager->updateDescriptorCreateInfos(obj.first + "UBO", res);
    }

    // we need to call this before pipeline creation and after all of the renderObjects are in the map
    m_pipelineManager->initDescriptorConfig();

    for (auto& obj: m_renderTheseObjects) { obj.second->createPipeline(m_pipelineManager); }

    createPrimaryCommandBuffer(m_primaryCommandBuffer);
    m_secondaryCommandBuffer.resize(m_renderTheseObjects.size());
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

std::vector<MATH::Vec2> VulkanRenderer::load2dVertexFile(const std::string& pathToFile)
{
    float x, y;
    std::vector<MATH::Vec2> vertices;
    std::ifstream infile(pathToFile);
    if (!infile.is_open())
        throw std::invalid_argument("load2dVertexFile cannot find file to load: " + pathToFile);
    while (infile >> x >> y)
    {
        vertices.push_back(MATH::Vec2{x, y});
    }
    return vertices;
}

std::vector<uint32_t> VulkanRenderer::loadIndexFile(const std::string& pathToFile)
{
    uint32_t x;
    std::vector<uint32_t> indices;
    std::ifstream infile(pathToFile);
    if (!infile.is_open())
        throw std::invalid_argument("loadIndexFile cannot find file to load: " + pathToFile);
    while (infile >> x)
    {
        indices.push_back(x);
    }
    return indices;
}

bool VulkanRenderer::createDeviceBuffer(const std::string& pathToFile, VkBuffer& buffer, VkBufferUsageFlags flags, VkDeviceMemory& bufferMemory, VkDeviceSize size, void* dataPointer)
{
    m_deviceHandler->createBuffer(size, flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

    void* data;
    vkMapMemory(m_deviceHandler->getLogicalDevice(), bufferMemory, 0, size, 0, &data);
    memcpy(data, dataPointer, (size_t)size);
    vkUnmapMemory(m_deviceHandler->getLogicalDevice(), bufferMemory);

    return true;
}

void VulkanRenderer::drawFrame()
{
    // update the UBOs to transfer the new data to the shaders
    for (auto& obj: m_renderTheseObjects)
    {
        obj.second->updateUBO(m_pipelineManager->getUBOAddress(obj.first + "UBO")[0]);
    }

    int i{0};
    for (auto& obj: m_renderTheseObjects)
    {
        m_deviceHandler->recordSecondaryCommandBufferStart(m_secondaryCommandBuffer[i], m_pipelineManager->getPipeline(obj.first + "Pipeline"));
        vkCmdBindDescriptorSets(m_secondaryCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager->getPipelineLayout(obj.first + "PipelineLayout"), 0, 1, &m_pipelineManager->getDescriptorSet(obj.first + "UBO"), 0, VK_NULL_HANDLE);
        obj.second->createCommandBuffer(m_secondaryCommandBuffer[i]);
        m_deviceHandler->recordSecondaryCommandBufferEnd(m_secondaryCommandBuffer[i]);
        i++;
    }

    // create the main command buffer with all of the secondary command buffers
    m_deviceHandler->recordPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);
    // after we update all of the UBOs we can render the frame
    m_deviceHandler->drawFrame(m_primaryCommandBuffer);
    // reset all of the variables so we can start and handle the next frame
    for (auto& obj: m_renderTheseObjects) { obj.second->resetFrameVariables(); }
}

void VulkanRenderer::vulkanRenderRect(const MATH::Vec2& position, const MATH::Vec2& size, const MATH::Vec4& color)
{
    MATH::Vec4 positionAndSize{position.x, position.y, size.x, size.y};
    auto rectangle = static_cast<Rectangle*>(m_renderTheseObjects["rectangle"]);
    rectangle->m_ubodata.positionAndSize[rectangle->m_rectangleCount] = MATH::Vec4{(positionAndSize.x /m_windowX - 1), (positionAndSize.y / m_windowY - 1), positionAndSize.z/(float)m_windowX, positionAndSize.w/(float)m_windowY};
    rectangle->m_ubodata.color[rectangle->m_rectangleCount] = color;
    if(rectangle->m_rectangleCount < 999)
        rectangle->m_rectangleCount += 1;
}

void VulkanRenderer::vulkanRenderShape2d(const std::string& nameVertex, const std::string &nameIndex, const MATH::Vec2& position, const MATH::Vec2& size, MATH::Vec4& color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount)
{
    auto shape = static_cast<Shape2d*>(m_renderTheseObjects["shape2d"]);
    MATH::Vec4 positionAndSize{position.x /m_windowX - 1, position.y /m_windowY - 1, size.x/(float)m_windowX, size.y/(float)m_windowY};
    shape->addShape2dToDraw(nameVertex, nameIndex, positionAndSize, color, vertexBuffer, indexBuffer, indexCount);
    if(shape->m_shapeCount < 999)//hardcoded max number of shapes; also in the uboSturct; TODO: change it to be variable
        shape->m_shapeCount += 1;
}

bool VulkanRenderer::load2dVertexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    auto vertices = load2dVertexFile(pathToFile);

    return createDeviceBuffer(pathToFile, buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferMemory, sizeof(vertices[0]) * vertices.size(), vertices.data());
}

bool VulkanRenderer::loadIndexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory, int& size)
{
    auto vertices = loadIndexFile(pathToFile);
    size = vertices.size();

    return createDeviceBuffer(pathToFile, buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bufferMemory, sizeof(vertices[0]) * vertices.size(), vertices.data());
}
