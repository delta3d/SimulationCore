varying vec3  ModelPosition;

void main(void)
{
    ModelPosition = vec3(gl_Vertex);

    gl_Position = ftransform();
}
