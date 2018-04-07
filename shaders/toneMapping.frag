#version 450 core

layout(binding=0) uniform sampler2D sceneTex;

out layout(location = 0)vec4 cColor;

in layout(location = 0) INTERFACE
{
	vec2 vTexCoord;
} vs_in;

vec4 linearToSrgb(in vec4 value)
{
	return mix(((1 + 0.055) * pow (value, vec4(1.0 / 2.4)) - vec4(0.055)),
	           (12.92 * value),
	           vec4(lessThanEqual(value, vec4(0.0031308))));
}

void main()
{
	vec4 sceneCol = texture(sceneTex, vs_in.vTexCoord);
	vec2 vDiff = vs_in.vTexCoord - vec2(0.5);
	cColor = (1.0 - dot(vDiff, vDiff)) * linearToSrgb(sceneCol);
	cColor = vec4(1.0);
}
