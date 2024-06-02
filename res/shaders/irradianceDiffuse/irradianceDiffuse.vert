#version 450
#extension GL_ARB_shader_viewport_layer_array : enable

// Inputs
layout (location = 0) in vec3 inPosition;

// Push constants
layout (push_constant) uniform PushConstants
{
    int index;
} pc;

// Outputs
layout (location = 0) out vec3 FragPos;

// Functions
mat4 lookAt(vec3 eye, vec3 at, vec3 up);
mat4 perspective(float fovy, float aspect, float near, float far);

// Constants
mat4 views[6] = {
    lookAt(vec3(0.0, 0.0, 0.0), vec3( 1.0,  0.0,  0.0), vec3(0.0, -1.0, 0.0)),
    lookAt(vec3(0.0, 0.0, 0.0), vec3(-1.0,  0.0,  0.0), vec3(0.0, -1.0, 0.0)),
    lookAt(vec3(0.0, 0.0, 0.0), vec3( 0.0,  1.0,  0.0), vec3(0.0,  0.0, 1.0)),
    lookAt(vec3(0.0, 0.0, 0.0), vec3( 0.0, -1.0,  0.0), vec3(0.0, 0.0, -1.0)),
    lookAt(vec3(0.0, 0.0, 0.0), vec3( 0.0,  0.0,  1.0), vec3(0.0, -1.0, 0.0)),
    lookAt(vec3(0.0, 0.0, 0.0), vec3( 0.0,  0.0, -1.0), vec3(0.0, -1.0, 0.0))
};

mat4 proj = perspective(90.0, 1.0, 0.1, 10.0);

void main()
{
    FragPos = inPosition;
    gl_Layer = pc.index;
    gl_Position = proj * views[pc.index] * vec4(inPosition, 1.0);
}


mat4 lookAt(vec3 eye, vec3 at, vec3 up) {

    vec3 zaxis = normalize(eye - at);           // The "forward" vector.
    vec3 xaxis = normalize(cross(up, zaxis));   // The "right" vector.
    vec3 yaxis = cross(zaxis, xaxis);           // The "up" vector.

    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    mat4 viewMatrix = {
        vec4(      xaxis.x,            yaxis.x,            zaxis.x,       0 ),
        vec4(      xaxis.y,            yaxis.y,            zaxis.y,       0 ),
        vec4(      xaxis.z,            yaxis.z,            zaxis.z,       0 ),
        vec4(-dot( xaxis, eye ), -dot( yaxis, eye ), -dot( zaxis, eye ),  1 )
    };

    return viewMatrix;
}

mat4 perspective(float fov, float aspect, float near, float far) {

    float tanHalfFov = tan(radians(fov) / 2.0);
    mat4 result = mat4(0.0);

    result[0][0] = 1.0 / (aspect * tanHalfFov);
    result[1][1] = 1.0 / tanHalfFov;
    result[2][2] = -(far + near) / (far - near);
    result[2][3] = -1.0;
    result[3][2] = -(2.0 * far * near) / (far - near);
    // result[3][2] = -(far * near) / (far - near);

    return result;
}