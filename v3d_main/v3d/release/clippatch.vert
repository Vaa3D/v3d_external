// for raycasting
#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerClr;

out vec3 Color;

uniform mat4 MVP;


void main()
{
	vec4 color4D = inverse(MVP) * vec4(VerClr, 1.0);
	Color = color4D.xyz / color4D.w;
    gl_Position = vec4(VerPos, 1.0);
}
