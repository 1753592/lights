#version 450

layout(binding = 0) uniform MVP
{
  mat4 model;
  mat4 proj;
  mat4 view;
  vec4 cam;
}
mvp;

layout(location = 0) in vec3 attr_pos;
layout(location = 1) in vec3 attr_norm;

layout(location = 0) out vec3 vp_pos;
layout(location = 1) out vec3 vp_norm;

void main(void)
{
  gl_Position = mvp.proj * mvp.view * mvp.model * vec4(attr_pos, 1.0);

  vp_pos = attr_pos;
  vp_norm = attr_norm;
}
