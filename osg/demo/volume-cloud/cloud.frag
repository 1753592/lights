#version 450 

uniform vec4 boxPos;
uniform vec4 boxLength;

uniform mat4 invModelViewProjectMatrix;

in vec3 localVertex;

layout(early_fragment_tests) in;

void main()
{
	vec3 camPos = vec3(
		gl_ModelViewMatrix[3][0],
		gl_ModelViewMatrix[3][1],
		gl_ModelViewMatrix[3][2]
	);
	vec3 dir = localVertex - camPos;
	gl_FragColor = vec4(normalize(dir), 1); 
}
