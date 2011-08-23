
uniform sampler2D texUnit0; // Diffuse texture

void main(void)
{
  vec4 diffuse = texture2D(texUnit0, gl_TexCoord[0].st);

  gl_FragColor = diffuse * gl_Color;
}
