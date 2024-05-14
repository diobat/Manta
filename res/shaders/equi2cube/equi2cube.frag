#version 450

// Inputs 
layout (location = 0) in vec3 FragPos;    

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D textureMap;

// Outputs
layout (location = 0) out vec3 FragColor;

// Constants
const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2 * PI) and 1 / (PI)

// Functions
vec2 SampleSphericalMap(vec3 v);

void main()
{
    vec2 uv = SampleSphericalMap(normalize(FragPos.xyz));
    vec3 color = texture(textureMap, uv).rgb;

    FragColor = color;
}

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
