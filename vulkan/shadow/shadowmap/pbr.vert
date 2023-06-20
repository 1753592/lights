#version 450

layout(binding = 0) uniform MatrixObject{
    mat4 proj;
    mat4 view;
    mat4 model;
    vec4 cam;
} ubo;

layout(location = 0) in vec3 attr_pos;
layout(location = 1) in vec3 attr_norm;

layout(location = 1) out vec3 vp_pos;
layout(location = 2) out vec3 vp_norm;

void main(void)
{
  gl_Position = ubo.proj * ubo.view * vec4(attr_pos, 1.0);

  vp_norm = attr_norm;
}
