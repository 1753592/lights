#version 450

//layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 vp_norm;
//layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 out_color;


//layout(set=1, binding = 2) uniform MaterialBufferObject{
//	float metallic;
//	float roughness;
//	float ao;
//	vec3 albedo;
//}
//mbo;


void main(void)
{
  out_color = vec4(vp_norm, 1);
}
