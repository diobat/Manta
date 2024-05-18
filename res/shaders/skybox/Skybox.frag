#version 450

// Uniforms
layout (set = 1, binding = 0) uniform samplerCube skyboxCube;

// Inputs
layout (location = 0) in vec3 TexCoords;

// Outputs
layout (location = 0) out vec4 FragColor;

void main()
{    
    vec3 color = textureLod(skyboxCube, TexCoords, 1.0).rgb; 
    FragColor = vec4(color,1.0);
}