#version 450 

uniform vec4 boxPos;
uniform vec4 boxLength;

uniform vec3 camPos;
uniform mat4 invModelViewProjectMatrix;

in vec3 localVertex;

layout(early_fragment_tests) in;

void main()
{
	vec3 dir = localVertex - camPos;
	gl_FragColor = vec4(normalize(dir), 1); 
}
