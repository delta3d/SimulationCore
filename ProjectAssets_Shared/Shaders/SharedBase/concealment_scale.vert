uniform vec4 ConcealDims;

vec3 getConcealmentScaledVertex(vec3 vert)
{
	return vert * ConcealDims.xyz;
}