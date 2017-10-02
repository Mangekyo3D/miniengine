#version 450 core

in INTERFACE
{
	vec3 vNormal;
} vs_in;

layout (std140, binding = 1) uniform LightingBlock
{
	vec4 cColor;
	vec3 lightDir;
} lighting;

out vec4 cColor;

vec4 linearToSrgb(in vec4 value)
{
	return mix(((1 + 0.55) * pow (value, vec4(1.0 / 2.4)) - vec4(0.55)),
	           (12.92 * value),
	           vec4(lessThanEqual(value, vec4(0.0031308))));
}

void main()
{
	vec3 normal = normalize(vs_in.vNormal);
	cColor = lighting.cColor * vec4 (dot(normal, lighting.lightDir));
	cColor = linearToSrgb(cColor);
//	cColor = 0.5f * vec4(normal, 1.0f) + vec4(0.5f);
}
