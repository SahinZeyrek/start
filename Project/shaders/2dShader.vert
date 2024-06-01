#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out float fragDeltaTime;
layout(push_constant) uniform Push 
{
	mat4 transform;
	mat4 normalMatrix;
} push;
layout(set = 0, binding = 0) uniform GlobalUBO
{
 mat4 projectionViewMatrix;
 mat4 inverseViewMatrix;
 vec3 cameraPosition;
 int renderMode;
 float deltaTime;
} ubo;
void main()
{
	gl_Position = vec4(position,0,1);

	fragColor = vec3(1.f,1.f,1.f);
	fragDeltaTime = ubo.deltaTime;
}