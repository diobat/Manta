#version 450

layout (set = 0, binding = 2) uniform sampler samp;
layout (set = 0, binding = 3) uniform texture2D texSampler[4096];

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstantObject {
    uint indexDiffuseTexture;
} pc;


void main() {
    outColor = texture(sampler2D(texSampler[pc.indexDiffuseTexture], samp), fragTexCoord);
}