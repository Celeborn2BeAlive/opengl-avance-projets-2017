#version 330

out vec3 fColor;

uniform sampler2D uSSAO;
uniform sampler2D uGAmbient;
uniform bool uBlur;

void main() {
	vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));
    
    float ssao = 0.0;

	if(uBlur)
	{
		for (int x = -1; x <= 1; ++x) 
		{
			for (int y = -1; y <= 1; ++y) 
			{
				vec2 offset = vec2(float(x), float(y));
				ssao += texelFetch(uSSAO, ivec2(gl_FragCoord.xy) + ivec2(offset), 0).r;
			}
		}
		ssao /= 9; 
	}
	else
	{
		ssao = texelFetch(uSSAO, ivec2(gl_FragCoord.xy), 0).r;
	}

    fColor = ka * ssao;
}  
