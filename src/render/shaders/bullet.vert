#version 450 core

in layout(location=0) vec4 ivPos;
in layout(location=1) vec2 ivTexCoord;

layout (std140, set = 0, binding = 0) uniform TransformBlock
{
	mat4 mViewFromWorld;
	mat4 mClipFromView;
	mat3 mNormal;
} transform;

out layout(location = 0) INTERFACE
{
	vec2 vTexCoord;
} vs_out;

void main()
{
	vec4 vViewPos = transform.mViewFromWorld * ivPos;
	gl_Position = transform.mClipFromView * vViewPos;
	// multiplying by the world matrix only works with uniform scaling
	vs_out.vTexCoord = ivTexCoord;
}
