#version 450

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

mat4 proj = perspective(radians(90.0), 1.0, 0.1, 10.0);

void main()
{
    FragPos = inPosition;
    gl_Position = proj * views[pc.index] * vec4(inPosition, 1.0);
}


mat4 lookAt(vec3 eye, vec3 at, vec3 up) {

    vec3 f = normalize(at - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    mat4 result;
    result[0] = vec4(s, 0.0);
    result[1] = vec4(u, 0.0);
    result[2] = vec4(-f, 0.0);
    result[3] = vec4(0.0, 0.0, 0.0, 1.0);

    result = transpose(result);
    result[3] = vec4(dot(-s, eye), dot(-u, eye), dot(f, eye), 1.0);

    return result;
}

mat4 perspective(float fovy, float aspect, float near, float far) {
    float tanHalfFovy = tan(radians(fovy) / 2.0);
    mat4 result = mat4(0.0);

    result[0][0] = 1.0 / (aspect * tanHalfFovy);
    result[1][1] = 1.0 / tanHalfFovy;
    result[2][2] = -(far + near) / (far - near);
    result[2][3] = -1.0;
    result[3][2] = -(2.0 * far * near) / (far - near);

    return result;
}