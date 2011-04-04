uniform vec4 color;

varying float vFog;

void main(void)
{
   gl_FragColor = vec4(mix(color.rgb, gl_Fog.color.rgb, vFog), color.a);  
}
