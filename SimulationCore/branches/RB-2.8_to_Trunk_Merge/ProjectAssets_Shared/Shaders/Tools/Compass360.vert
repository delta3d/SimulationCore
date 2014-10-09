uniform vec2 cameraFOVAngleMinMax;
uniform vec4 screenRect;

const float TEXTURE_WIDTH_DEGREES = 90.0;
const float DEGREE_RATIO = 1.0/360.0;
const float TEXTURE_DEGREE_RATIO = DEGREE_RATIO * 360.0/TEXTURE_WIDTH_DEGREES; 
const float TEXTURE_OFFSET = 2.5;

void main(void)
{
   vec4 pos = ftransform();
   pos.x = screenRect.x + pos.x * screenRect.z - (1.0 - screenRect.z);
   pos.y = screenRect.y + pos.y * screenRect.w - (1.0 - screenRect.w);
   gl_Position = pos;
   
   vec4 uv = gl_MultiTexCoord0;
   
   vec2 camFOV = vec2(cameraFOVAngleMinMax.x, cameraFOVAngleMinMax.y);
   
   float fov = camFOV.y - camFOV.x;
   
   // Shift the horizontal UV coordinates.
   vec2 normalizedAngles = cameraFOVAngleMinMax * TEXTURE_DEGREE_RATIO;
   
   // Angle limits are flipped to compensate for the reversed transform of the camera.
   float normalizedFov = normalizedAngles.x - normalizedAngles.y;
   normalizedAngles.x -= (TEXTURE_OFFSET + 90 - fov)*TEXTURE_DEGREE_RATIO;
   
   // Shift the texture coordinates.
   // Multiply x by -1 to undo texture flipping from previous step.
   float offset = uv.x*normalizedFov + normalizedAngles.x;
   
   gl_TexCoord[0].x = -offset;
   gl_TexCoord[0].y = uv.y*0.25;
}
