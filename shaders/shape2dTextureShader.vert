#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 texPos;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexPos;

layout(binding = 0) uniform uboData {
    vec4 positionAndSize[1000];
    vec4 color[1000];
} ubo;

void main() {
    gl_Position = vec4(
        ubo.positionAndSize[gl_InstanceIndex].x + inPosition.x * ubo.positionAndSize[gl_InstanceIndex].z,
        ubo.positionAndSize[gl_InstanceIndex].y + inPosition.y * ubo.positionAndSize[gl_InstanceIndex].w,
        0.0,
        1.0);
    fragColor = ubo.color[gl_InstanceIndex].xyz;
    fragTexPos = texPos;
}