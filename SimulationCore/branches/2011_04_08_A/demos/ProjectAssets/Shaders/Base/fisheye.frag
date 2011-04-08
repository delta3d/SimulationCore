uniform sampler2D baseTexture;
uniform vec2      lensFocus;

//a fish-eye lens effect for the compass.
//
// X = ((d+1)(delta/r))/(d*(delta/r)+1)*r 
// X = distance of the pixel from the focus of the lens
// r = maximum distance from the focus
// delta = distance of the original pixel from the focus
// d = distortion factor 

// compute coordinate to take the texture from
vec2 ComputeNewLoc(in vec2 coord, in vec2 focus)
{
    float radius = .125;
    float d = 2.0;
    float diff = distance(coord,focus);

    float delta = diff*radius;
    delta /=( (d+1.0)*radius - diff*d);

   vec2 dir = normalize(coord - focus);

   return delta*dir+focus;	
}

//mask anything outside of the radius
float ComputeMask(in vec2 coord, in vec2 focus)
{
     float diff = distance(coord,focus);
     float radius = .125;
     diff =  radius- diff;
     //if we're outside of the radius, we're negative and should return 0
     float mask = sign(diff);
     mask = step(0.0, mask);
     return mask;
}

void main(void)
{
    vec2 newCoord = ComputeNewLoc(gl_TexCoord[0].st, lensFocus);    

    vec4 tempColor = texture2D(baseTexture, newCoord );
    tempColor.w = ComputeMask(gl_TexCoord[0].st, lensFocus);

    gl_FragColor = tempColor;

}
