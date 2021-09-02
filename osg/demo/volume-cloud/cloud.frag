#version 450 compatibility 

uniform vec4 boxPos;
uniform vec4 boxLength;

uniform vec4 screenSize;

uniform vec3 camPos;
uniform mat4 invModelViewProjectMatrix;

uniform sampler2D depTex;

in vec3 localVertex;

layout(early_fragment_tests) in;

void main()
{
	vec2 uv = gl_FragCoord.xy / screenSize.xy;
	float dep = texture(depTex, uv).r;
	vec4 depPos = vec4(uv * 2 - 1.0, 0, 1);
	depPos.z = dep * 2 - 1.0;
	depPos = invModelViewProjectMatrix * depPos;
	vec3 depPoz = depPos.xyz / depPos.w;

	vec3 dir = normalize(localVertex - camPos);
	float sum = 0;
	vec3 minbox = boxPos.xyz - boxLength.xyz;
	minbox -= 0.00001;
	vec3 maxbox = boxPos.xyz + boxLength.xyz;
	maxbox += 0.00001;
	const int count = 256;
	const float step = length(boxLength) / count;
	for (int i = 0; i < count; i++) {
		vec3 tmpPos = localVertex + dir * i * step;

		if (dot(tmpPos - depPoz, dir) > 0) {
			//gl_FragColor = vec4(1, 0, 0, 1);
			//return;
			break;
		}

		if (tmpPos.x < minbox.y || tmpPos.x > maxbox.x ||
			tmpPos.y < minbox.y || tmpPos.y > maxbox.y ||
			tmpPos.z < minbox.z || tmpPos.z > maxbox.z)
			break;
		sum += 0.01;
	}
	gl_FragColor = vec4(sum);
	//gl_FragColor = vec4(1, 0, 0, 1);
}
