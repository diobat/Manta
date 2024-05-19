#version 450

// Uniforms
layout (set = 0, binding = 2) uniform sampler samp;
layout (set = 0, binding = 3) uniform texture2D texDiffuse[4096];

// Inputs
layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;

// Outputs
layout (location = 0) out vec4 outColor;

// Push constants
layout (push_constant) uniform PushConstantObject {
    layout(offset = 128) uint indexDiffuseTexture;
} pc;

void main() {
    outColor = texture(sampler2D(texDiffuse[pc.indexDiffuseTexture], samp), fragTexCoord);
}