#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cam;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;

void main(void)
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	fragPos = inPosition;
	normal = inNormal;
	uv = inUV;
}
