#version 450

//layout(binding = 0) uniform UniformBufferObject {
//    vec3 cam;
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;

layout(location = 0) in vec3 attr_pos;

void main(void)
{
  //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
  //gl_Position.y = -gl_Position.y;
  gl_Position = vec4(attr_pos, 1);
}
