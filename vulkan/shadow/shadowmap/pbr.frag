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

layout(binding = 1) uniform Light
{
  vec4 light_pos;
  vec4 light_color;
} light;

layout(binding = 2) uniform Material {
  float ao;
  float metallic;
  float roughness;
  vec4 albedo;
} material;


const float pi = 3.14159265359;

vec3 fresnel_schlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_GGX(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float ndot_h = max(dot(n, h), 0.0);
	float ndot_h2 = ndot_h * ndot_h;

	float denom = ndot_h2 * (a2 - 1.0) + 1.0;
	denom = pi * denom * denom;

	return a2 / max(denom, 0.0000001);
}

float schlick_GGX(float ndotv, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float denom = ndotv * (1.0 - k) + k;

	return ndotv / denom;
}

float smith_Geometry(vec3 n, vec3 v, vec3 l, float roughness)
{
	float ndotv = max(dot(n, v), 0.0);
	float ndotl = max(dot(n, l), 0.0);
	float ggx2 = schlick_GGX(ndotv, roughness);
	float ggx1 = schlick_GGX(ndotl, roughness);

	return ggx1 * ggx2;
}


void main(void)
{
  vec3 mate_albedo = material.albedo.rgb;
  float mate_roughness = material.roughness;
  float mate_metallic = material.metallic;
  float mate_ao = material.ao;

  vec3 cam = ubo.cam.xyz;
  vec3 n = normalize(vp_norm);
  vec3 v = normalize(cam - vp_pos);

  vec3 f0 = vec3(0.04);
  f0 = mix(f0, mate_albedo, mate_metallic);
  vec3 lo = vec3(0.0);

  vec3 light_pos = light.light_pos.xyz;
  vec3 light_color = light.light_color.rgb;

  vec3 l = normalize(light_pos - vp_pos);
  vec3 h = normalize(v + l);
  float distance = length(light_pos - vp_pos);
  float attenuation = 1.0 / (distance * distance);
  vec3 radiance = light_color * attenuation;

  frag_color = vec4(l, 1);
  return;

  float nv = distribution_GGX(n, h, mate_roughness);

  float gv = smith_Geometry(n, v, l, mate_roughness);
  vec3 fv = fresnel_schlick(clamp(dot(h, v), 0.0, 1.0), f0);

  vec3 nominator = nv * gv * fv;
  float denominator = 4 * max(dot(n, v), 0) * max(dot(n, l), 0.0);
  vec3 specular = nominator / max(denominator, 0.000001);

  vec3 ks = fv;
  vec3 kd = vec3(1.0) - ks;
  kd *= (1.0 - mate_metallic);

  float ndotl = max(dot(n, l), 0);

  lo = (kd * mate_albedo / pi + specular) * radiance * ndotl;
  

  vec3 ambient = vec3(0.03) * mate_albedo * mate_ao;
  vec3 color = ambient + lo;

  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  frag_color = vec4(color, 1.0);
}
