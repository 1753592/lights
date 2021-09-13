#version 430 compatibility

layout(location = 0) uniform sampler2D tex0;
layout(location = 1) uniform sampler2D tex1;
layout(location = 2) uniform sampler2D tex2;
layout(location = 3) uniform sampler2D tex3;

void main()
{
	gl_FragColor = texture(tex1, gl_TexCoord[0].st);
}
