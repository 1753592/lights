#version 330 compatibility

out vec3 normal;
out vec3 fragPos;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	gl_TexCoord[0] = gl_MultiTexCoord0;

	normal = gl_Normal;
	fragPos = gl_Vertex.xyz;
}
