
#ifdef TEX3D
uniform sampler3D volume;
#else
uniform sampler2D volume; // 3D scaled fly brain image intensities
#endif

uniform sampler2D colormap; // Color/HDR/gamma correction for each data channel
uniform sampler2D neuronVisibility; // On/off visibility of each neuron fragment 256x256 2D texture
uniform sampler3D neuronLabel; // 3D volume locations of neuron indices

uniform vec4 channel; // Lookup table for channel index into colormap.  When would this ever change?
uniform int blend_mode;
uniform int format_bgra;

varying float clipDistance[MaxClipPlanes];

//////////////////////////////////////
void main()
{
    // Apply user clip planes (slowly) in fragment shader because useless ATI/Mac combo is broken for years at a time.
    for (int p = 0; p < MaxClipPlanes; ++p) {
        if (clipDistance[p] < 0) {
            discard;
        }
    }

    // Use 3D texture coordinate in texture unit 3 to get neuron label position
    float neuronIx = texture3D(neuronLabel, gl_TexCoord[3].xyz).r;
    // prepare to multiply neuron colors by visibility value; so neurons will toggle on/off
    // To sidestep texture dimension limits, neuronVisibility is stored in a 2D 256x256 texture,
    // instead of a 1D 1x65536 texture.
    float iIx = floor(neuronIx * 65535.1);
    float visY = floor(iIx / 256.0);
    float visX = (iIx - 256.0 * visY) / 256.0;
    visY = visY / 256.0;
    vec2 visCoord = vec2(visX, visY);
    vec4 visSel = texture2D(neuronVisibility, visCoord);
    float visibility = visSel.x;
    float selectedness = visSel.y;

    // vColor is the scaled intensity of 3 data channels(in r,g,b) plus nc82 reference(in a)
#ifdef TEX3D
        vec4 vColor = texture3D(volume, gl_TexCoord[0].xyz);
        // vec4 vColor = vec4(0, 1, 0, 1);
#else
        vec4 vColor = texture2D(volume, gl_TexCoord[0].xy);
#endif

    if (format_bgra == 1)
    {
        vColor.rgba = vColor.bgra;
    }

    // C1...C4 are the colorized, gamma-ed, HDR-ed versions of the channel data
#ifdef TEX_LOD // only works in vertex shader!!!
        vec4 C1 = texture2DLod(colormap, vec2(vColor.r, 0), channel.x); // 0.0
        vec4 C2 = texture2DLod(colormap, vec2(vColor.g, 0), 0.25);
        vec4 C3 = texture2DLod(colormap, vec2(vColor.b, 0), 0.50);
        vec4 C4 = texture2DLod(colormap, vec2(vColor.a, 0), 0.75);
#else
        vec4 C1 = texture2D(colormap, vec2(vColor.r, channel.x)); // 0.0
        vec4 C2 = texture2D(colormap, vec2(vColor.g, channel.y));
        vec4 C3 = texture2D(colormap, vec2(vColor.b, channel.z));
        vec4 C4 = texture2D(colormap, vec2(vColor.a, channel.w)); // ref is unaffected by neuron visibility
#endif

        // Turn off data channels if visibility is toggled off
        C1.a *= visibility;
        C2.a *= visibility;
        C3.a *= visibility;
        // Reference remains visible, even if neuron is toggled off.

        // aC1..aC4 are colors scaled by alpha component
	vec3 aC1 = C1.rgb * C1.a;
	vec3 aC2 = C2.rgb * C2.a;
	vec3 aC3 = C3.rgb * C3.a;
        vec3 aC4 = C4.rgb * C4.a;

        float dataAlphaMax = max(C1.a, max(C2.a, C3.a));
        float alphaMax = max(dataAlphaMax, 0.25 * C4.a); // put some extra transparency into reference/nc82 channel

        vec3 neuronColor = aC1 + aC2 + aC3 + aC4;
        float alpha = alphaMax;

        // Selected neurons are (mostly) opaque, and have a high brightness
        // For numerical stability, compute alpha part first
        float aScale1 = selectedness * 0.9 / (dataAlphaMax + 0.01); // avoid NaN
        vec3 selectedColor = aScale1 * (aC1 + aC2 + aC3); // no reference channel in selected
        float selectedAlpha = selectedness * (dataAlphaMax + 0.5); // selected neurons are mostly opaque

        if (blend_mode == 1) {
            // Alpha blending mode (as opposed to maximum intensity projection)
            // Maximize brightness of color.  All dimming is done via alpha.
            float aScale2 = 1.0 / max(dataAlphaMax, C4.a);
            neuronColor = C1.rgb * (C1.a * aScale2) + C2.rgb * (C2.a * aScale2) + C3.rgb * (C3.a * aScale2) + C4.rgb * (C4.a * aScale2);
        }

        float Amean = (C1.a + C2.a + C3.a + C4.a)/4.0;
        float Asum = C1.a + C2.a + C3.a + C4.a;
        float Amax = max(C1.a, max(C2.a, max(C3.a, C4.a)));

	vec4 oColor;

        // This blending was not working on linux (selectedColor had NaN?)
        float a2 = 1.0 - selectedness; // complement of selectedness
        oColor.rgb = mix(neuronColor, selectedColor, selectedness);
        // oColor.rgb = selectedColor + neuronColor * a2; // OK, now that NaN is elided
        alpha = selectedAlpha + alpha * a2;
        oColor.a = pow(alpha, 1.5); // reduce effect of small intensities

	gl_FragColor = gl_Color * oColor; // modulated by color_proxy
}
