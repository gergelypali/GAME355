#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include "Vector.h"

namespace DESCRIPTOR
{
    struct descriptorSetLayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    };

    struct descriptorSetInfo
    {
        VkDescriptorSetLayout layout;
        VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        VkDescriptorSet set;
        // these last two are needed to update the allocated set with real buffer data
        VkWriteDescriptorSet writeInfo{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        VkDescriptorBufferInfo bufferInfo;
    };

    struct descriptorPoolInfo
    {
        VkDescriptorPool pool;
        VkDescriptorPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        std::vector<VkDescriptorPoolSize> poolSizes;
    };
}

namespace PIPELINE
{
    struct pipelineInfo
    {
        std::string vertShaderPath{""};
        std::string fragShaderPath{""};
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        std::vector<VkDynamicState> dynamicStates;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        VkPipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        VkPipelineLayout pipelineLayout{};
        VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        VkPipeline pipeline;
    };

    struct pipelineLayoutInfo
    {
        VkPipelineLayoutCreateInfo layoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        VkPipelineLayout layout;
        std::vector<VkDescriptorSetLayout> layouts;
    };
}

namespace BUFFER
{
    struct vec4PushConstant
    {
        MATH::Vec4 vector;
    };

    // uniform buffer struct
    struct uboData
    {
        MATH::Vec4 vector[1000];
    };
}

#endif