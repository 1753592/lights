#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec3 cam;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main(void)
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position.y = -gl_Position.y;

	fragPos = inPosition;
	fragNormal = inNormal;
	fragUV = inUV;
}
