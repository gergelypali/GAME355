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
            binding0.stride = sizeof(position);

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
            input0.offset = 0;

            res.push_back(input0);

            return res;
        }
    };

    struct shape2dwithtexture : baseVertex
    {
        MATH::Vec2 position{};
        MATH::Vec2 texPos{};

        std::vector<VkVertexInputBindingDescription> getBindingDescriptor() override
        {
            std::vector<VkVertexInputBindingDescription> res{};

            VkVertexInputBindingDescription binding0{};
            binding0.binding = 0;
            binding0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            binding0.stride = sizeof(position) + sizeof(texPos);

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
            input0.offset = 0;

            res.push_back(input0);

            VkVertexInputAttributeDescription input1{};
            input1.binding = 0;
            input1.location = 1;
            input1.format = VK_FORMAT_R32G32_SFLOAT;
            input1.offset = sizeof(position);

            res.push_back(input1);

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
        std::vector<VkVertexInputBindingDescription> vertexInputBindings;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
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

    struct descriptorLayoutInfo
    {
        VkDescriptorSetLayout layout{};
        VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    };
}

namespace BUFFER
{
    // the maxSize is a tricky question: most of the GPUs it is 256 bytes
    // but to be like "99%" compatible it should be 128 bytes which is
    // 32 floats
    struct uintPushConstant
    {
        uint32_t number;
    };

    struct shape2dPushConstant
    {
        uint32_t notTexture{0};
        MATH::Vec2 position{};
        MATH::Vec2 size{};
        MATH::Vec3 color{};
    };

    struct vec4PushConstant
    {
        MATH::Vec4 vector{};
    };

    // uniform buffer struct
    struct uboData
    {
        MATH::Vec4 vector[1000];
    };

    struct shape2dUboData
    {
        MATH::Vec4 positionAndSize[2048];
        MATH::Vec4 color[2048];
    };
}

namespace TEXTURE
{
    struct textureData
    {
        VkImage image{};
        VkDeviceMemory imageMemory{};
        VkImageView imageView{};
        VkDescriptorSet set{};
        int width{0};
        int height{0};
    };
}

#endif