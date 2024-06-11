#version 450

// Inputs 
layout (location = 0) in vec3 FragPos;    

// Uniforms
layout (set = 0, binding = 0) uniform samplerCube textureMap;

// Push Constants
layout (push_constant) uniform PushConstants
{
    layout(offset = 128) float roughness;
}pc;

// Constants
const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 4096u;

// Outputs
layout (location = 0) out vec4 FragColor;

// Functions
float vanDerCorput(uint bits);
vec2 hammersley(uint i, uint N);
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness);


// Main
void main()
{
    vec3 N = normalize(FragPos);
    vec3 R = N;
    vec3 V = R;

    float total_weight = 0.0;
    vec3 preFilteredColor = vec3(0.0);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);  // Hammersley sequence for importance sampling
        vec3 H = importanceSampleGGX(Xi, N, pc.roughness);  // Importance sampling with GGX
        vec3 L = normalize(2.0 * dot(V, H) * H - V);    // Calculate the light vector

        float NdotL = max(dot(N, L), 0.0);

        preFilteredColor += texture(textureMap, L).rgb * NdotL;
        total_weight += NdotL;

    }
    preFilteredColor = preFilteredColor / total_weight;
    
    FragColor = vec4(preFilteredColor, 1.0);
}

// Function Definitions
float vanDerCorput(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), vanDerCorput(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, N));
    vec3 tangentY = cross(N, tangentX);

    // Tangent to world space
    return tangentX * H.x + tangentY * H.y + N * H.z;
}