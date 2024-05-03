#version 450

// Inputs
layout (location = 0) in vec3 inPosition;

// Uniforms
layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Outputs
layout (location = 0) out vec3 FragPos;

void main()
{
    FragPos = inPosition;
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
}