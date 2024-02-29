#version 450

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform uboData {
    vec4 vector[1000];
} ubo;

layout(push_constant) uniform constants {
    vec4 position;
} PushConstants;

vec2 vertexPositions[6] = vec2[](
    vec2(0.1, 0.1),
    vec2(0.1, -0.1),
    vec2(-0.1, 0.1),
    vec2(-0.1, 0.1),
    vec2(0.1, -0.1),
    vec2(-0.1, -0.1)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(ubo.vector[gl_InstanceIndex].xy + vertexPositions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}