#version 330


out vec4 vp_clr;

void main()
{
  vp_clr = gl_Color;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_PointSize = 128;
}