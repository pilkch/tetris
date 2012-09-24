#version 130

uniform sampler2D texUnit0; // Base texture

smooth in vec2 vertOutTexCoord0;
smooth in vec4 vertOutColour;

out vec4 fragmentColor;

void main()
{
  vec4 diffuse = texture2D(texUnit0, vertOutTexCoord0);

  fragmentColor = diffuse * vertOutColour;
}
