#version 400



in vec3 EntryPoint;
in vec4 ExitPointCoord;

uniform sampler2D ExitPoints;
uniform sampler3D VolumeTex;
uniform sampler1D TransferFunc;  
uniform float     StepSize;
uniform vec2      ScreenSize;
uniform int channel;
uniform int clipmode;
uniform vec2      ImageSettings; // x = contrast, y= brightness
uniform vec3 clippoint;
uniform vec3 clipnormal;
uniform float clipwidth;
layout (location = 0) out vec4 FragColor;
void main()
{

    vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st/ScreenSize).xyz;
    if (EntryPoint == exitPoint)
    	//background need no raycasting
    	discard;
    vec3 dir = exitPoint - EntryPoint;
    float len = length(dir); // the length from front to back is calculated and used to terminate the ray
	float stepsizenew = len/256;
    vec3 deltaDir = normalize(dir) * StepSize;
	//vec3 deltaDir = dir / 256;
    float deltaDirLen = length(deltaDir);
    vec3 voxelCoord = EntryPoint;
    vec4 colorAcum = vec4(0.0); // The dest color
    float alphaAcum = 0.0;                // The  dest alpha for blending
    vec4 intensity;
    float lengthAcum = 0.0;
    vec4 colorSample; // The src color 
    float alphaSample; // The src alpha
    // backgroundColor
    vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.0);
	
	bool needclip = true;
	// vec4 color4D = inverse(MVP) * vec4(VerClr, 1.0);
    for(int i = 0; i < 1600; i++)
    {
		intensity = texture(VolumeTex, voxelCoord);
		voxelCoord += deltaDir;
		lengthAcum += deltaDirLen;
		if (clipmode == 1)
		{
			vec3 nordir = voxelCoord - clippoint;
			if (dot(clipnormal, nordir) < 0)
			{
				needclip = false;
			}
			else
			{
				if (lengthAcum >= len)
				{
					colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;
					break; // terminate if opacity > 1 or the ray is outside the volume
				}
				else if (colorAcum.a > 1.0)
				{
					colorAcum.a = 1.0;
					break;
				}
				continue;
			}
		}
		// else if (clipmode == 2)
		// {
		// 	vec3 slabpointup = clippoint + clipwidth * clipnormal;
		// 	vec3 slabpointdown = clippoint - clipwidth * clipnormal;
		// 	vec3 nordirup = voxelCoord - slabpointup;
		// 	vec3 nordirdown = voxelCoord - slabpointdown;
		// 	if (((dot(clipnormal, nordirup) < 0) && (dot(clipnormal, nordirdown) > 0)) || ((dot(clipnormal, nordirup) > 0) && (dot(clipnormal, nordirdown) < 0)))
		// 	{
		// 		needclip = false;
		// 	}
		// 	else
		// 	{
		// 		if (lengthAcum >= len)
		// 		{
		// 			colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;
		// 			break; // terminate if opacity > 1 or the ray is outside the volume
		// 		}
		// 		else if (colorAcum.a > 1.0)
		// 		{
		// 			colorAcum.a = 1.0;
		// 			break;
		// 		}
		// 		continue;
		// 	}
			
		// }
		else
		{
			needclip = false;
		}

		if (channel == 0)
		{
			if (colorAcum.r < intensity.x)
			{
				//float cri = colorAcum.r + intensity.x*intensity.x*(1-colorAcum.a);
				//float ari = colorAcum.a + intensity.x*(1-colorAcum.a);
				//colorAcum = vec4(cri,cri,cri,ari);
				colorAcum = vec4(intensity.x);
			}
		}
		else if (channel == 1)
		{
			if (colorAcum.r < intensity.x)
			{
				colorAcum = vec4(intensity.x, colorAcum.g, colorAcum.b, 1.0);
			}
			if (colorAcum.g < intensity.y)
				colorAcum = vec4(colorAcum.r, intensity.y, colorAcum.b, 1.0);
			if (colorAcum.b < intensity.z)
				colorAcum = vec4(colorAcum.r, colorAcum.g, intensity.z, 1.0);
		}
		else if (channel == 2)
		{
			if (colorAcum.r < intensity.x)
				colorAcum = vec4(intensity.x, 0.0, 0.0, 1.0);
		}
		else if (channel == 3)
		{
			if (colorAcum.g < intensity.y)
				colorAcum = vec4(0.0, intensity.y, 0.0, 1.0);
		}
		else if (channel == 4)
		{
			if (colorAcum.b < intensity.z)
				colorAcum = vec4(0.0, 0.0, intensity.z, 1.0);
		}

		if (lengthAcum >= len)
		{
			colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;
			break; // terminate if opacity > 1 or the ray is outside the volume
		}
		else if (colorAcum.a > 1.0)
		{
			colorAcum.a = 1.0;
			break;
		}
	}
    colorAcum.a = 1.0;

	if(needclip)
	{
		discard;                
	}
    colorAcum.rgb = colorAcum.rgb * ImageSettings.x;// + ImageSettings.y;
    if(colorAcum.r>1) colorAcum.r =1;
    if(colorAcum.g>1) colorAcum.g =1;
    if(colorAcum.b>1) colorAcum.b =1;

	//brightness supression
	if (ImageSettings.y < 0.5) // temporarily use ImageSettings.y for this
	{
		if(colorAcum.r>0.4) colorAcum.r = 0.4;
		//else if(colorAcum.r>0.2) colorAcum.r *= 1.5;
		if(colorAcum.g>0.4) colorAcum.g = 0.4;
		//else if(colorAcum.g>0.2) colorAcum.g *= 1.5;
		if(colorAcum.b>0.4) colorAcum.b = 0.4;
		//else if(colorAcum.b>0.2) colorAcum.b *= 1.5;
	}
    
    FragColor = colorAcum;   
}
