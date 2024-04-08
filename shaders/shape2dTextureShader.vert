#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 texPos;

layout(location = 0) out vec2 fragTexPos;

layout(binding = 0) uniform uboData {
    vec4 positionAndSize[2048];
    vec4 color[2048];
} ubo;

void main() {
    fragTexPos = texPos;
    gl_Position = vec4(
        ubo.positionAndSize[gl_InstanceIndex].x + inPosition.x * ubo.positionAndSize[gl_InstanceIndex].z,
        ubo.positionAndSize[gl_InstanceIndex].y + inPosition.y * ubo.positionAndSize[gl_InstanceIndex].w,
        0.0,
        1.0);
}