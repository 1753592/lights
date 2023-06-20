#version 450

layout(location = 1) in vec3 vp_pos;
layout(location = 2) in vec3 vp_norm;
//layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 frag_color;

layout(binding = 0) uniform MatrixObject{
    mat4 proj;
    mat4 view;
    mat4 model;
    vec4 cam;
} ubo;

const float pi = 3.14159265359;

void main(void)
{
  frag_color = vec4(1, 0, 0,1);
}
