#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;
layout(push_constant) uniform Push 
{
	mat4 transform;
	mat4 normalMatrix;
} push;
void main()
{
	gl_Position = vec4(position,0,1);

	fragColor = color;
}