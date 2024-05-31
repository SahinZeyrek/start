#version 450
// VERTEX STRUCT 
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;

// DATA TO SEND TO FRAGMENT SHADER

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUV;
layout (location = 3) out vec3 fragTangent;
layout (location = 4) out vec3 fragCameraPosition;
layout (location = 5) out vec4 fragVertexPosition;
layout (location = 6) out int fragRenderMode;

layout(set = 0, binding = 0) uniform GlobalUBO
{
 mat4 projectionViewMatrix;
 mat4 inverseViewMatrix;
 vec3 cameraPosition;
 int renderMode;
} ubo;



layout(push_constant) uniform Push 
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	fragColor = inColor;
	fragUV = inUV;
	fragCameraPosition = ubo.cameraPosition;
	fragNormal =  normalize(mat3(push.normalMatrix) * inNormal);
	fragTangent = normalize(mat3(push.normalMatrix) * inTangent);
	fragRenderMode = ubo.renderMode;
	fragVertexPosition = push.modelMatrix * vec4(position,1.0);
	gl_Position = ubo.projectionViewMatrix * fragVertexPosition;

}