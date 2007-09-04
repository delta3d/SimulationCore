// Note - You can add about 2 more varying before it fails to compile on the 7950 Go NVidia cards

//varying vec2 testXY; // see note in fragment shader
varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 weights; // [0] is blur, [1] is medium, and [2] is fine

varying float fog;
varying float distance;

//This vertex shader is meant to perform the per vertex operations of per pixel lighting
//using a single directional light source.
void main()
{
   const float MAX_DISTANCE = 750.0;

   // make normal and light dir in world space like the gl_vertex
   vNormal = normalize(gl_Normal);
   vLightDirection = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));

   //Pass the texture coordinate on through.
   gl_TexCoord[0] = gl_MultiTexCoord0;   
   gl_FogFragCoord = gl_FogCoord;
   
   // reduce the effect of the detail mapping as we move away from a normal that points
   // straight up.  This prevents the shader from applying to the sides of buildings 
   // since their normal has almost no z component.
   float shaderWeight = abs(vNormal.z); // * shaderInEffect;


   //Calculate the distance from this vertex to the camera.
   vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
   vec3 ecPosition3 = vec3(ecPosition) / ecPosition.w;
   distance = length(ecPosition3);

      
   //We want the detail texture to fade away as the distance from the camera increases.
   //The detail and fine blend use curved weights based on how close they are to the camera
   weights[0] = (1.0 - clamp(distance/MAX_DISTANCE,0.0,1.0)) * shaderWeight;
   float blurSquared = weights[0] * weights[0];
   weights[1] = clamp(blurSquared * 1.3, 0.0, 1.0); // more affect closer up.
   weights[2] = sin(weights[0]) * blurSquared; // fine only effects when VERY close.
    
   // New code uses the fog distance directly to discard the pixel.
   float fog_start = gl_Fog.end * 0.20;
   float fog_end = gl_Fog.end;   
        
   // Note - the best equation is: 2 ^ (-8 * dist * dist) where dist = 0 at fog_start and 1 at fog_end. 
   // This gives a nice S sort of curve with fog = 1 at x = 0 and fog = 0 at X = 1.
   // Here, we just use linear.
   //float fogXValue = clamp((distance - fog_start) * (1/(fog_end - fog_start)), 0.0, 1.0); 
   //fog = pow(2.0, -8 * fogXValue * fogXValue);
   fog = clamp((distance - fog_start) / (fog_end - fog_start), 0.0, 1.0); 


   //Compute the final vertex position in clip space.
   gl_Position = ftransform();
}
