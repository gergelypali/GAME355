#version 450

layout(location = 0) in vec2 fragTexPos;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexPos);
}