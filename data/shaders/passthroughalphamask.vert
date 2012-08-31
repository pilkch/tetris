
uniform mat4 matModelViewProjection;

void main()
{
  gl_FrontColor = gl_Color;
  gl_Position = matModelViewProjection * gl_Vertex;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}

