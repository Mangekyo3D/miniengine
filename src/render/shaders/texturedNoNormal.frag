#version 450 core

in layout(location = 0) INTERFACE
{
	vec2 vTexCoord;
} vs_in;

layout(set = 1, binding=0) uniform sampler2D diffuseTex;

out layout(location = 0) vec4 cColor;

void main()
{
	cColor = texture(diffuseTex, vs_in.vTexCoord);
}
