#version 450

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform uboData {
    vec4 positionAndSize[1000];
    vec4 color[1000];
} ubo;

vec2 vertexPositions[6] = vec2[](
    vec2(1, 1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(-1, 1),
    vec2(1, -1),
    vec2(-1, -1)
);

void main() {
    gl_Position = vec4(
        ubo.positionAndSize[gl_InstanceIndex].x + vertexPositions[gl_VertexIndex].x * ubo.positionAndSize[gl_InstanceIndex].z,
        ubo.positionAndSize[gl_InstanceIndex].y + vertexPositions[gl_VertexIndex].y * ubo.positionAndSize[gl_InstanceIndex].w,
        0.0,
        1.0);
    fragColor = ubo.color[gl_InstanceIndex].xyz;
}