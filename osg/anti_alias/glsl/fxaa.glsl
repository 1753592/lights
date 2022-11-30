
layout(binding = 0) uniform FX_TextureInfo
{
	vec2i			tex_size;
	Sampler2D		tex;

} fx_texture;

float luminace(vec3 v)
{
	return v.x * 0.2126729 + v.y * 0.7151522 + v.z * 0.0721750;
}

vec4 fxaa_quality(Sampler2D sampler, vec2 coord)
{
	
}