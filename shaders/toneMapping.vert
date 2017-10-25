#version 450 core

in layout(location=0) vec4 ivPos;

out INTERFACE
{
	vec2 vTexCoord;
} vs_out;


void main()
{
	gl_Position = ivPos;
	vs_out.vTexCoord = ivPos.xy * 0.5 + vec2(0.5);
}
