#version 450
 
// Inputs
layout (location = 0) in vec3 inPosition;

layout (location = 2) in vec2 inTexCoords;

// Outputs
layout (location = 0) out vec2 texCoords;

void main()
{
    texCoords = vec2(inPosition.x, -inPosition.y)/2.0 + 0.5;
	gl_Position = vec4(inPosition, 1.0);
}