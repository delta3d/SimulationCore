varying float vDistance;
varying vec4 vColor;
varying vec4 vTex0;
varying vec4 vTex1;

void calculateDistance(mat4, vec4, out float);

void main(void)
{
    //
    // Transform 2 positions by the modelview
    //

    // last component of color contains segment length, to be
    // used in finding the opposite segment vertex position.
    vec4 endpos = vec4(0.0,0.0,0.0,1.0);
    endpos.xyz = gl_Vertex.xyz + (gl_Normal * gl_Color.w);

    calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);

    vec4 posstart = gl_ModelViewMatrix * gl_Vertex;
    vec4 posend   = gl_ModelViewMatrix * endpos;
    
    // Unit vector between EYE and CENTER of the line
    vec3  middlepoint = normalize((posstart.xyz + posend.xyz)*0.5);
    
    // Unit vector of the line direction
    vec3 lineoffset = posend.xyz - posstart.xyz; 
    vec3 linedir = normalize(lineoffset); 
    float sqlinelength = dot(lineoffset, lineoffset);
    
    // Dot-product in order to compute the texture coef
    float texcoef = abs( dot(linedir, middlepoint) );
    
    // Change texture coef depending on line length: y=(Sz/(l^2))(x-1)+1
    // NOTE: Color holds the line width in z, either + or -
    float lineWidth = abs( gl_Color.z );
    texcoef = max( ((texcoef - 1.0)*(sqlinelength / (lineWidth*0.5))) + 1.0, 0.0 );

    //
    // model-view + projection on start and end points
    //

    posstart = gl_ModelViewProjectionMatrix * gl_Vertex;
    posend   = gl_ModelViewProjectionMatrix * endpos;
    
    // Perspective transform to get projected point
    vec2 startpos2d = posstart.xy / posstart.w;
    vec2 endpos2d   = posend.xy / posend.w;
    
    // Vector between these 2 points
    vec2 linedir2d = normalize(startpos2d - endpos2d);
    
    // Move corners with radius0 and radius1
    posstart.xy = ((texcoef * lineWidth) * linedir2d.xy) + posstart.xy; // horizontal

    // NOTE: Color contains a signed width at z
    linedir2d = gl_Color.z * linedir2d;

    posstart.x = posstart.x + linedir2d.y; // vertical x
    posstart.y = posstart.y - linedir2d.x; // vertical y
    gl_Position = posstart;

    //
    // Compute tex-coords depending on texcoef
    //
    float row = 0.0;
    float column = 0.0;
    float fraction = 0.0;
    float tmpTexcoef = texcoef;
    float blend;

    tmpTexcoef = min(15.0/16.0, tmpTexcoef); // We don't want more than 15/16
    tmpTexcoef *= 4.0;
    fraction = mod( tmpTexcoef, 1.0);
    row = tmpTexcoef - fraction;

    fraction *= 4.0;
    blend = mod(fraction, 1.0); // Fractional part of fraction * 4 is the blend factor
    column = fraction - blend;

    // Offset texture coordinates
    vTex0.xy = gl_Color.xy + vec2( 0.25*column, 0.25*row );
    vTex0.zw = vec2(0.0,1.0);

	//
	// Now get the second texture coord : increment
	//

        tmpTexcoef = min(texcoef + (1.0/16.0), 15.0/16.0); // We don't want more than 15/16
        tmpTexcoef *= 4.0;
        fraction = mod( tmpTexcoef, 1.0);
        row = tmpTexcoef - fraction;

        fraction *= 4.0;
        column = floor(fraction);


        // Offset texture coordinates
        vTex1.xy = gl_Color.xy + vec2( 0.25*column, 0.25*row );
        vTex1.zw = vec2(0.0,1.0);

        vColor = vec4(blend);
 }
