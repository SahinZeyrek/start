#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
 mat4 projectionViewMatrix;
 vec3 directionToLight;
} ubo;


layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push 
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;
void main()
{
	const vec3 lightDirection = normalize(vec3(1.0,-3.0,-1.0));

	gl_Position = ubo.projectionViewMatrix *  push.modelMatrix * vec4(position,1.0);
	vec3 temp = normalize(normal);
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * temp);

	float lightIntensity = max(dot(normalWorldSpace, ubo.directionToLight), 0);
	
	fragColor =  lightIntensity * color;
}