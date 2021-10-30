
uniform vec4 clipPlane[MaxClipPlanes];
// Radeon/Mac brokenness of custom clip planes requires slow slow fragment level manual clipping
varying float clipDistance[MaxClipPlanes];

void main()
{
    gl_FrontColor = gl_Color;

    // gl_ClipDistance[0] = dot(clipPlane[0], vertex); // sadly GLSL 1.3 syntax not yet available on Mac
    // Why is apple always years and years behind on Java and OpenGL? GLSL 1.3 was released in 2008.
    //
    // gl_ClipVertex = gl_Vertex; // no effect on Radeon
    for (int p = 0; p < MaxClipPlanes; ++p) {
        clipDistance[p] = dot(gl_Vertex, clipPlane[p]);
    }

    // We use 4 textures, so need to pass along 4 texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord2;
    gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord3;

    // Use standard transformation to recapitulate fixed-function OpenGL pipeline
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
