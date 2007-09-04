// This Shader is dependent on the following files
//   perpixel_lighting_fragment_functions.glsl
//   fragment_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes


uniform sampler2D diffuseTexture;
uniform sampler2D detailTexture;

varying vec3 vNormal;
varying vec3 vHalfVec;
varying vec3 vLightDir;

void create2DTexture(sampler2D, vec2, out vec4);
void initilizePerPixelVariables(out vec3, inout vec3, inout vec3, out float);
void computeColor(vec3, vec3, inout vec3);

/**
 * This is the fragment shader to a simple set of shaders that calculates
 * per pixel lighting assuming one directional light source.  It also
 * combines a diffuse texture located in texture unit 0 with a detail map
 * located in texture unit 1.
 */
void main()
{
   vec4 diffuseColor;
   create2DTexture(diffuseTexture, gl_TexCoord[0].st, diffuseColor);
   
   
   vec3 detailColor = vec3(texture2D(detailTexture,gl_TexCoord[0].st * 8.0)) * 0.5;

   vec3 color;
   vec3 normal;
   vec3 lightDir;
   float diffuseContrib;
   initilizePerPixelVariables(color, vNormal, vLightDir, diffuseContrib);      
   
   if (diffuseContrib > 0.0)
   {
      //Diffuse + Texture + DetailMap
      color += (vec3(gl_LightSource[0].diffuse) * (vec3(diffuseColor) * 0.5) + detailColor) * diffuseContrib;

      //Specular...
      computeColor(vHalfVec, vNormal, color);
   }

   gl_FragColor = vec4(color,diffuseColor.a);
}
