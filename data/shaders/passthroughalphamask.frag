
uniform sampler2D texUnit0; // Base texture

const float tolerance = 0.1;

void main()
{
  vec4 diffuse = texture2D(texUnit0, gl_TexCoord[0].st);
  if (diffuse.a < tolerance) discard;

  gl_FragColor = diffuse * gl_Color;
}

