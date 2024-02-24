#include "VulkanScene1.h"
#include "VulkanRenderer.h"

void VulkanScene1::init()
{

}

void VulkanScene1::endScene()
{

}

void VulkanScene1::update()
{
    sRender();
    m_currentFrame++;
}

void VulkanScene1::sRender()
{
    m_ge->vulkanRenderer()->drawFrame();
}

void VulkanScene1::sDoAction(const Action& action)
{

}
