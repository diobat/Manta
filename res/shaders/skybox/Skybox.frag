#version 450

// Uniforms
layout (set = 0, binding = 1) uniform sampler samp;
layout (set = 0, binding = 2) uniform textureCube texCubemap[64];

// Inputs
layout (location = 0) in vec3 TexCoords;

// Outputs
layout (location = 0) out vec4 FragColor;

layout (push_constant) uniform PushConstantObject {
    layout(offset = 128) uint indexCubeTexture;
} pc;

void main()
{    
    vec4 color = texture(samplerCube(texCubemap[pc.indexCubeTexture], samp), TexCoords);
    FragColor = color;
}