#version 450 core

in INTERFACE
{
	vec3 vNormal;
	vec2 vTexCoord;
} vs_in;

layout (std140, binding = 1) uniform LightingBlock
{
	vec4 cColor;
	vec3 lightDir;
} lighting;

layout(binding=0) uniform sampler2D diffuseTex;

out vec4 cColor;

void main()
{
	vec3 normal = normalize(vs_in.vNormal);
	vec4 cDiffuse = texture(diffuseTex, vs_in.vTexCoord);
	cColor = lighting.cColor * vec4(dot(normal, lighting.lightDir)) * cDiffuse;
}
