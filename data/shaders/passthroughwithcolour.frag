
uniform sampler2D texUnit0; // Diffuse texture

void main(void)
{
  vec3 diffuse = texture2D(texUnit0, gl_TexCoord[0].st).rgb;

  gl_FragColor = vec4(diffuse.rgb * gl_Color.rgb, 1.0);
}
