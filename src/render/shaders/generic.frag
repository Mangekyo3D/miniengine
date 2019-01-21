#version 450 core

in layout(location = 0) INTERFACE
{
	vec3 vNormal;
} vs_in;

layout (std140, set = 0, binding = 1) uniform LightingBlock
{
	vec4 cColor;
	vec3 lightDir;
} lighting;

out layout (location = 0) vec4 cColor;

void main()
{
	vec3 normal = normalize(vs_in.vNormal);
	cColor = lighting.cColor * vec4 (dot(normal, lighting.lightDir));
//	cColor = 0.5f * vec4(normal, 1.0f) + vec4(0.5f);
}
