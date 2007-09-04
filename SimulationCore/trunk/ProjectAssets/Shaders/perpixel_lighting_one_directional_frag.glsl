// This Shader is dependent on the following files
//   perpixel_lighting_fragment_functions.glsl
//   fragment_functions.glsl

uniform sampler2D diffuseTexture;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vHalfVec;

void create2DTexture(sampler2D, vec2, out vec4);
void initilizePerPixelVariables(out vec3, inout vec3, inout vec3, out float);
void computeColor(vec3, vec3, inout vec3);

void main()
{
   vec4 diffuseColor = texture2D(diffuseTexture,gl_TexCoord[0].st);
   create2DTexture(diffuseTexture, gl_TexCoord[0].st, diffuseColor);

   vec3 color;
   vec3 normal;
   vec3 lightDir;
   float diffuseContrib;
   initilizePerPixelVariables(color, vNormal, vLightDir, diffuseContrib);

   if (diffuseContrib > 0.0)
   {
      //Diffuse...
      color += (vec3(gl_LightSource[0].diffuse) * vec3(diffuseColor)) * diffuseContrib;

      //Specular...
      computeColor(vHalfVec, vNormal, color);
   }

   gl_FragColor = vec4(color,diffuseColor.a);
}
