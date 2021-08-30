#version 330

in vec2 vertOut;

uniform float time;
uniform ivec2 screenSize;

uniform int cate;
uniform float repNum;
uniform vec2 uv;

vec3 hash32(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yxz + 33.33);
	return fract((p3.xxy + p3.yzz) * p3.zyx);
}

vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yxz + 33.33);
	return fract((p3.xxy + p3.yxx) * p3.zyx);

}

float worley2d(in vec2 x, float u, float v)
{
	vec2 p = floor(x);
	vec2 f = fract(x);

	float k = 1.0 + 63.0 * pow(1.0 - v, 4.0);
	float va = 0.0;
	float wt = 0.0;
	for (int j = -2; j <= 2; j++)
		for (int i = -2; i <= 2; i++) {
			vec2  g = vec2(float(i), float(j));
			vec3  o = hash32(p + g) * vec3(u, u, 1.0);
			vec2  r = g - f + o.xy;
			float d = dot(r, r);
			float w = pow(1.0 - smoothstep(0.0, 1.414, sqrt(d)), k);
			va += w * o.z;
			wt += w;
		}

	return va / wt;
}

void main()
{
	vec2 p = gl_FragCoord.xy / screenSize.y;
	vec3 p3 = vec3(vec3(p, time * 0.025));

	float value;

	value = worley2d(p * vec2(64), 0, 0);

	//value *= smoothstep(0.0, 0.005, abs(0.6 - p.x)); 

	gl_FragColor = vec4(vec3(value), 1.0);
}
