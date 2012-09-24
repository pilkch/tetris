#version 330

uniform mat4 matModelViewProjection;

#define POSITION 0
#define COLOUR 1
#define TEXCOORD0 2

layout(location = POSITION) in vec2 position;
layout(location = COLOUR) in vec4 colour;
layout(location = TEXCOORD0) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec4 vertOutColour;
smooth out vec2 vertOutTexCoord0;

void main()
{
  gl_Position = matModelViewProjection * vec4(position.xy, 0.0, 1.0);
  vertOutColour = colour;
  vertOutTexCoord0 = texCoord0;
}
