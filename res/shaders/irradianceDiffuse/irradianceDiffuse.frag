#version 450

// Inputs 
layout (location = 0) in vec3 FragPos;    

// Uniforms
layout (set = 0, binding = 0) uniform samplerCube textureMap;

// Constants
const float PI = 3.14159265359;

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{
    vec3 N = normalize(FragPos);

    vec3 irradiance = vec3(0.0);

    // Calculate world space vector base
    vec3 up = vec3(0.0, 1.0, 0.0); 
    vec3 right = cross(up, N);
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) // Longitude all around the sphere
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) // Latitude from bottom to top of the sphere
        {
            // Hop from spherical to cartesian coordinates
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // And then use the vector base to get the world space 
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(textureMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}