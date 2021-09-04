#version 330

in vec2 vertOut;

uniform float time;
uniform ivec2 screenSize;

uniform int cate;
uniform float repNum;
uniform vec2 uv;

float worley2d_test(vec2, float, float);

void main()
{
	vec2 p = gl_FragCoord.xy / screenSize.y;
	vec3 p3 = vec3(vec3(p, time * 0.025));

	float value;

	if (cate == 0) {
		value = worley2d_test(p * vec2(repNum), uv.x * sin(time), uv.y);
	} else {
		//value = worley_fractal(p * vec2(repNum), uv.x * sin(time), uv.y);
	}

	//value *= smoothstep(0.0, 0.005, abs(0.6 - p.x)); 

	gl_FragColor = vec4(vec3(value), 1.0);
}
