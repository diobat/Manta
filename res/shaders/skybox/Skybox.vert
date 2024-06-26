#version 450

// Uniforms
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Input
layout (location = 0) in vec3 inPosition;
// Output
layout (location = 0) out vec3 TexCoords;

void main()
{
    TexCoords = inPosition;
    vec4 pos = ubo.proj * mat4(mat3(ubo.view)) * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;
}