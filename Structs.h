#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include "Vector.h"

namespace VERTEX
{
    struct baseVertex
    {
        virtual std::vector<VkVertexInputBindingDescription> getBindingDescriptor() = 0;
        virtual std::vector<VkVertexInputAttributeDescription> getAttributeDescriptor() = 0;
    };

    struct baseRectangle : baseVertex
    {
        MATH::Vec2 position;

        std::vector<VkVertexInputBindingDescription> getBindingDescriptor() override
        {
            std::vector<VkVertexInputBindingDescription> res{};

            VkVertexInputBindingDescription binding0{};
            binding0.binding = 0;
            binding0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            binding0.stride = sizeof(baseRectangle);

            res.push_back(binding0);

            return res;
        }
        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptor() override
        {
            std::vector<VkVertexInputAttributeDescription> res{};

            VkVertexInputAttributeDescription input0{};
            input0.binding = 0;
            input0.location = 0;
            input0.format = VK_FORMAT_R32G32_SFLOAT;
            input0.offset = offsetof(baseRectangle, position);

            res.push_back(input0);

            return res;
        }
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

    struct descriptorCreateInfo
    {
        VkDescriptorType type;
        VkShaderStageFlags shaderStageFlags;
        VkDeviceSize bufferSize;
        uint8_t numberOfMaxSets{0};
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

    struct rectangleUboData
    {
        MATH::Vec4 positionAndSize[1000];
        MATH::Vec4 color[1000];
    };
}

#endif