#version 450 compatibility 

uniform vec4 boxPos;
uniform vec4 boxLength;

uniform vec4 screenSize;

uniform vec3 camPos;
uniform mat4 invModelViewProjectMatrix;

uniform sampler2D depTex;
uniform sampler3D noiseTex;

in vec3 localVertex;

//layout(early_fragment_tests) in;

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

	vec3 startPos = localVertex;
	if (camPos.x > minbox.x && camPos.x < maxbox.x &&
		camPos.y > minbox.y && camPos.y < maxbox.y &&
		camPos.z > minbox.z && camPos.z < maxbox.z)
	{
		startPos = camPos + dir * 0.001;
		gl_FragColor = vec4(0, 1, 0, 1);
		return;
		discard;
	}

	for (int i = 0; i < count; i++) {
		vec3 tmpPos = startPos + dir * i * step;

		if (dot(tmpPos - depPoz, dir) > 0) {
			gl_FragColor = vec4(1, 0, 0, 1);
			return;
			break;
		}

		if (tmpPos.x < minbox.x || tmpPos.x > maxbox.x ||
			tmpPos.y < minbox.y || tmpPos.y > maxbox.y ||
			tmpPos.z < minbox.z || tmpPos.z > maxbox.z)
		{
			//gl_FragColor = vec4(1, 0, 0, 1);
			//return;
			break;
		}

		vec3 uvw = (tmpPos - minbox) / boxLength.xyz / 2.0;
		sum += texture(noiseTex, uvw).r * 0.02;
		//break;
	}
	gl_FragColor = vec4(sum);
	//gl_FragColor = vec4(1, 0, 0, 1);
}
