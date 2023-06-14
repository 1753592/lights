#version 450

layout(binding = 0) uniform UBO{
    mat4 proj;
    mat4 view;
    mat4 model;
    vec4 cam;
} ubo;

layout(location = 0) in vec3 attr_pos;

void main(void)
{
  gl_Position = ubo.proj * ubo.view * vec4(attr_pos, 1.0);
  //gl_Position.y = -gl_Position.y;
  //gl_Position = vec4(attr_pos, 1);
}
