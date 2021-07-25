#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    vec3 camPos;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform Lights{
	vec3 lightPositions[4];
	vec3 lightColors[4];
}
lights;

layout(set=1, binding = 2) uniform MaterialBufferObject{
	float metallic;
	float roughness;
	float ao;
	vec3 albedo;
}
mbo;

const float pi = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float ndotH = max(dot(n, h), 0.0);
	float ndotH2 = ndotH * ndotH;

	float denom = ndotH2 * (a2 - 1.0) + 1.0;
	denom = pi * denom * denom;

	return a2 / max(denom, 0.0000001);
}

float schlickGGX(float ndotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float denom = ndotV * (1.0 - k) + k;

	return ndotV / denom;
}

float smithGeometry(vec3 n, vec3 v, vec3 l, float roughness)
{
	float ndotV = max(dot(n, v), 0.0);
	float ndotL = max(dot(n, l), 0.0);
	float ggx2 = schlickGGX(ndotV, roughness);
	float ggx1 = schlickGGX(ndotL, roughness);

	return ggx1 * ggx2;
}

void main(void)
{
	vec3 n = normalize(fragNormal);
	vec3 v = normalize(ubo.camPos - fragPos);

	vec3 f0 = vec3(0.04);
	f0 = mix(f0, mbo.albedo, mbo.metallic);
	vec3 lo = vec3(0.0);
	for (int i = 0; i < 4; i++) {
		vec3 l = normalize(lights.lightPositions[i] - fragPos);
		vec3 h = normalize(v + l);
		float distance = length(lights.lightPositions[i] - fragPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lights.lightColors[i] * attenuation;

		float nv = distributionGGX(n, h, mbo.roughness);
		float gv = smithGeometry(n, v, l, mbo.roughness);
		vec3 fv = fresnelSchlick(clamp(dot(h, v), 0.0, 1.0), f0);

		vec3 nominator = nv * gv * fv;
		float denominator = 4 * max(dot(n, v), 0) * max(dot(n, l), 0.0);
		vec3 specular = nominator / max(denominator, 0.000001);

		vec3 ks = fv; vec3 kd = vec3(1.0) - ks;
		kd *= (1.0 - mbo.metallic);

		float ndotL = max(dot(n, l), 0);

		lo += (kd * mbo.albedo / pi + specular) * radiance * ndotL;
	}

	vec3 ambient = vec3(0.03) * mbo.albedo * mbo.ao;
	vec3 color = ambient + lo;

	//color = color / (color + vec3(1.0));
	//color = pow(color, vec3(1.0 / 2.2));

	outColor = vec4(color, 1.0);
	//outColor = vec4(lights.lightColors[0], 1.0);
}
