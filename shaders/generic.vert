#version 450 core

in layout(location=0) vec4 ivPos;
in layout(location=1) vec3 ivNormal;
in layout(location=2) mat4 imWorldFromModel;

layout (std140, binding = 0) uniform TransformBlock
{
	mat4 mViewFromWorld;
	mat4 mClipFromView;
	mat3 mNormal;
} transform;

out INTERFACE
{
	vec3 vNormal;
} vs_out;

void main()
{
	vec4 vWorldPos = imWorldFromModel * ivPos;
	vec4 vViewPos = transform.mViewFromWorld * vWorldPos;

	gl_Position = transform.mClipFromView * vViewPos;
	vs_out.vNormal = transform.mNormal * ivNormal;
}
