#version 330

in vec2 vertOut;

uniform float time;
uniform ivec2 screenSize;

uniform int cate;
uniform float repNum;
uniform vec2 offset;

vec3 random3(vec3 p)
{
	//float j = 4096.0 * sin(dot(p, vec3(17.0, 59.4, 15.0)));
	//vec3 r;
	//r.z = fract(512.0 * j);
	//j *= .125;
	//r.x = fract(512.0 * j);
	//j *= .125;
	//r.y = fract(512.0 * j);
	//return r - 0.5;

	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yxz + 33.33);
	return fract((p3.xxy + p3.yzz) * p3.zyx) - 0.5;
}

const float F3 = 0.3333333;
const float G3 = 0.1666667;

float simplex3d(vec3 p)
{
	vec3 s = floor(p + dot(p, vec3(F3)));
	vec3 x = p - s + dot(s, vec3(G3));

	vec3 e = step(vec3(0.0), x - x.yzx);
	vec3 i1 = e * (1.0 - e.zxy);
	vec3 i2 = 1.0 - e.zxy * (1.0 - e);

	vec3 x1 = x - i1 + G3;
	vec3 x2 = x - i2 + 2.0 * G3;
	vec3 x3 = x - 1.0 + 3.0 * G3;

	vec4 w, d;

	w.x = dot(x, x);
	w.y = dot(x1, x1);
	w.z = dot(x2, x2);
	w.w = dot(x3, x3);

	w = max(0.6 - w, 0.0);

	d.x = dot(random3(s), x);
	d.y = dot(random3(s + i1), x1);
	d.z = dot(random3(s + i2), x2);
	d.w = dot(random3(s + 1.0), x3);

	w *= w;
	w *= w;
	d *= w;

	return dot(d, vec4(52.0));
}

const mat3 rot1 = mat3(-0.37, 0.36, 0.85, -0.14, -0.93, 0.34, 0.92, 0.01, 0.4);
const mat3 rot2 = mat3(-0.55, -0.39, 0.74, 0.33, -0.91, -0.24, 0.77, 0.12, 0.63);
const mat3 rot3 = mat3(-0.71, 0.52, -0.47, -0.08, -0.72, -0.68, -0.7, -0.45, 0.56);

float simplex3d_fractal(vec3 m)
{
	return 0.5333333 * simplex3d(m * rot1)
		+ 0.2666667 * simplex3d(2.0 * m * rot2)
		+ 0.1333333 * simplex3d(4.0 * m * rot3)
		+ 0.0666667 * simplex3d(8.0 * m);
}

float simplex3d_fractal_normal(vec3 m)
{
	float ret = 0;
	for (int i = 0; i < 5; i++) {
		float ratio = pow(2.0, i);
		ret += 1 / ratio * simplex3d(m * ratio);
	}
	return ret * 0.68;// * 0.51612;
}

float simplex3d_fractal_abs(vec3 m)
{
	float ret = 0;
	for (int i = 0; i < 5; i++) {
		float ratio = pow(2.0, i);
		ret += 1 / ratio * abs(simplex3d(m * ratio));
	}
	return ret * 0.68;
}

float simplex3d_fractal_abs_sin(vec3 m)
{
	float ret = simplex3d_fractal_abs(m);
	return  sin(ret * 2.5 + m.x * 0.2);
}

void main()
{
	vec2 p = gl_FragCoord.xy / screenSize;
	vec3 p3 = vec3(vec3(p, time * 0.025));

	float value;

	if (cate == 0) {
		value = simplex3d(p3 * repNum + vec3(offset, 0));
		value = 0.5 + 0.5 * value;
	} else if (cate == 1) {
		value = simplex3d_fractal(p3 * repNum + vec3(offset, 0));
		value = 0.5 + 0.5 * value;
	} else if (cate == 2) {
		value = simplex3d_fractal_normal(p3 * repNum + vec3(offset, 0));
		value = 0.5 + 0.5 * value;
	} else if (cate == 3)
		value = simplex3d_fractal_abs(p3 * repNum + vec3(offset, 0));
	else if (cate == 4) {
		value = simplex3d_fractal_abs_sin(p3 * repNum + vec3(offset, 0));
		value = 0.5 + 0.5 * value;
	}

	//value *= smoothstep(0.0, 0.005, abs(0.6 - p.x)); 

	gl_FragColor = vec4(vec3(value), 1.0);
}
