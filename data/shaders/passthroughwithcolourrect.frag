#version 330

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texUnit0; // Base texture

smooth in vec2 vertOutTexCoord0;
smooth in vec4 vertOutColour;

out vec4 fragmentColor;

void main()
{
  vec4 diffuse = texture2DRect(texUnit0, vertOutTexCoord0);

  fragmentColor = diffuse * vertOutColour;
}
