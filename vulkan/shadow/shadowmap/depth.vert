#version 450

layout(binding = 0) uniform MatrixObject{
    mat4 model;
    mat4 proj;
    mat4 view;
    vec4 cam;
} ubo;

layout(location = 0) in vec3 attr_pos;
layout(location = 0) out vec3 vp_pos;

void main(void)
{
  gl_Position = ubo.proj * ubo.view * vec4(attr_pos, 1.0);
}