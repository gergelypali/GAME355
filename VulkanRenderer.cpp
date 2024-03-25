#include "VulkanRenderer.h"
#include "DeviceHandler.h"
#include "PipelineManager.h"
#include <math.h>
#include "Logger.h"
#include "Shape2d.h"

#include <fstream>

VulkanRenderer::VulkanRenderer(SDL_Window* window)
    : m_window(window)
{
    m_deviceHandler = new DeviceHandler(window);
    m_pipelineManager = new PipelineManager(m_deviceHandler);

    m_windowX = m_deviceHandler->getWindowSize().width / 2;
    m_windowY = m_deviceHandler->getWindowSize().height / 2;

    Shape2d* shape2d = new Shape2d();
    m_renderTheseObjects.insert({"shape2d", shape2d});

    for (auto& obj: m_renderTheseObjects)
    {
        obj.second->createUboBuffer(m_deviceHandler);
        obj.second->createPipeline(m_pipelineManager);
    }

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

void VulkanRenderer::createPrimaryCommandBuffer(VkCommandBuffer &buffer)
{
    m_deviceHandler->createCommandBuffer(buffer, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void VulkanRenderer::createSecondaryCommandBuffer(std::vector<VkCommandBuffer>& buffer)
{
    m_deviceHandler->createCommandBuffer(buffer, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY);
}

std::vector<MATH::Vec4> VulkanRenderer::load2dVertexFile(const std::string& pathToFile)
{
    float x, y, tx, ty;
    std::vector<MATH::Vec4> vertices;
    std::ifstream infile(pathToFile);
    if (!infile.is_open())
        throw std::invalid_argument("load2dVertexFile cannot find file to load: " + pathToFile);
    while (infile >> x >> y >> tx >> ty)
    {
        vertices.push_back({MATH::Vec4{x, y, tx, ty}});
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

bool VulkanRenderer::createAndCopyDataToGPUSideBuffer(VkBuffer& buffer, VkBufferUsageFlags flags, VkDeviceMemory& bufferMemory, VkDeviceSize size, void* dataPointer)
{
    // create a temporary stagingBuffer that will be the sourceBuffer to copy from to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    m_deviceHandler->createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory
        );

    void* data;
    vkMapMemory(m_deviceHandler->getLogicalDevice(), stagingMemory, 0, size, 0, &data);
    memcpy(data, dataPointer, (size_t)size);
    vkUnmapMemory(m_deviceHandler->getLogicalDevice(), stagingMemory);

    m_deviceHandler->createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer,
        bufferMemory
        );

    m_deviceHandler->copyBufferToGPU(stagingBuffer, buffer, size);

    vkDestroyBuffer(m_deviceHandler->getLogicalDevice(), stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_deviceHandler->getLogicalDevice(), stagingMemory, VK_NULL_HANDLE);

    return true;
}

bool VulkanRenderer::createAndCopyDataToCPUSideBuffer(VkBuffer& buffer, VkBufferUsageFlags flags, VkDeviceMemory& bufferMemory, VkDeviceSize size, void* dataPointer)
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
        obj.second->updateUBO();
    }

    int i{0};
    for (auto& obj: m_renderTheseObjects)
    {
        m_deviceHandler->recordRenderSecondaryCommandBufferStart(m_secondaryCommandBuffer[i]);
        obj.second->createCommandBuffer(m_secondaryCommandBuffer[i]);
        m_deviceHandler->recordEndCommandBuffer(m_secondaryCommandBuffer[i]);
        i++;
    }

    // create the main command buffer with all of the secondary command buffers
    m_deviceHandler->recordRenderPrimaryCommandBuffer(m_primaryCommandBuffer, m_secondaryCommandBuffer);
    // after we update all of the UBOs we can render the frame
    m_deviceHandler->drawFrame(m_primaryCommandBuffer);
    // reset all of the variables so we can start and handle the next frame
    for (auto& obj: m_renderTheseObjects) { obj.second->resetFrameVariables(); }
}

void VulkanRenderer::vulkanRenderRect(const MATH::Vec2& position, const MATH::Vec2& size, const MATH::Vec4& color)
{
    //TODO: use the shape2d for rendering rectangle
}

void VulkanRenderer::vulkanRenderShape2d(const std::string& nameVertex, const std::string &nameIndex, const MATH::Vec2& position, const MATH::Vec2& size, MATH::Vec4& color, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, int indexCount)
{
    auto shape = static_cast<Shape2d*>(m_renderTheseObjects["shape2d"]);
    MATH::Vec4 positionAndSize{position.x /m_windowX - 1, position.y /m_windowY - 1, size.x/(float)m_windowX, size.y/(float)m_windowY};
    shape->addShape2dToDraw(nameVertex, nameIndex, positionAndSize, color, vertexBuffer, indexBuffer, indexCount);
    if(shape->m_shapeCount < 999)//hardcoded max number of shapes; also in the uboSturct; TODO: change it to be variable
        shape->m_shapeCount += 1;
}

void VulkanRenderer::vulkanRenderShape2dWithTexture(const std::string &nameVertex, const std::string &nameIndex, const std::string &nameTexture, const MATH::Vec2 &position, const MATH::Vec2 &size, VkDescriptorSet &set, VkBuffer &vertexBuffer, VkBuffer &indexBuffer, int indexCount)
{
    auto shape = static_cast<Shape2d*>(m_renderTheseObjects["shape2d"]);
    MATH::Vec4 positionAndSize{position.x /m_windowX - 1, position.y /m_windowY - 1, size.x/(float)m_windowX, size.y/(float)m_windowY};
    shape->addShape2dToDraw(nameVertex, nameIndex, nameTexture, positionAndSize, set, vertexBuffer, indexBuffer, indexCount);
    if(shape->m_shapeCount < 999)//hardcoded max number of shapes; also in the uboSturct; TODO: change it to be variable
        shape->m_shapeCount += 1;
}

bool VulkanRenderer::load2dVertexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    auto vertices = load2dVertexFile(pathToFile);

    return createAndCopyDataToGPUSideBuffer(buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferMemory, sizeof(vertices[0]) * vertices.size(), vertices.data());
}

bool VulkanRenderer::loadIndexBuffer(const std::string& pathToFile, VkBuffer& buffer, VkDeviceMemory& bufferMemory, int& size)
{
    auto vertices = loadIndexFile(pathToFile);
    size = vertices.size();

    return createAndCopyDataToGPUSideBuffer(buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bufferMemory, sizeof(vertices[0]) * vertices.size(), vertices.data());
}

void VulkanRenderer::freeBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    m_deviceHandler->destroyBuffer(buffer, bufferMemory);
}

void VulkanRenderer::loadTexture(textureData& textureData, void* pixelData, int size, uint32_t w, uint32_t h)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    m_deviceHandler->createBuffer(
        (VkDeviceSize) size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory
        );

    void* data;
    vkMapMemory(m_deviceHandler->getLogicalDevice(), stagingMemory, 0, size, 0, &data);
    memcpy(data, pixelData, (size_t)size);
    vkUnmapMemory(m_deviceHandler->getLogicalDevice(), stagingMemory);

    m_deviceHandler->createImage(
        w,
        h,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureData.image,
        textureData.imageMemory
        );

    m_deviceHandler->changeImageLayout(
        textureData.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );
    m_deviceHandler->copyBufferToImage(stagingBuffer, textureData.image, w, h);
    m_deviceHandler->changeImageLayout(
        textureData.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    );

    m_deviceHandler->destroyBuffer(stagingBuffer, stagingMemory);

    m_deviceHandler->createImageView(textureData.imageView, textureData.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    m_pipelineManager->createSamplerDescriptorSet(textureData.set, textureData.imageView);
}

void VulkanRenderer::destroyImage(VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView)
{
    m_deviceHandler->destroyImageView(imageView);
    m_deviceHandler->destroyImage(image, imageMemory);
}
