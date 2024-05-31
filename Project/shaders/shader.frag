#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragUV;
layout (location = 3) in vec3 fragTangent;
layout (location = 4) in vec3 fragCameraPosition;
layout (location = 5) in vec4 fragVertexPosition;
layout (location = 6) in flat int fragRenderMode;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D glossSampler;
layout(binding = 4) uniform sampler2D specularSampler;

layout(push_constant) uniform Push 
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float PI = 3.141592f;

const vec3 LightDirection = vec3(0.5f, 0.8f, -2.5f);
const vec3 AmbientLight =	vec3(0.03f, 0.03f, 0.03f);

vec4 ShadePixel(vec3 normal,vec2 uv,vec3 tangent,vec3 cameraPos,vec4 vertexWorldPos)
{
	vec3  albedo		= texture(albedoSampler,  fragUV).rgb;
	vec3  sampledNormal	= texture(normalSampler,  fragUV).rgb;
	float glossiness	= texture(glossSampler,	  fragUV).r;
	vec3  specular		= texture(specularSampler,fragUV).rgb;

	// CALCULATE NORMAL - TANGENT SPACE
	sampledNormal = normalize(sampledNormal * 2.f - 1.f);
	vec3 binormal = normalize(cross(normal,tangent));
	mat3 tangentSpaceAxis = mat3(tangent,binormal,normal);

	sampledNormal = normalize(tangentSpaceAxis * sampledNormal);
	  // Vectors
    vec3 lightDir =		normalize(LightDirection);
    vec3 viewDir  =		normalize(cameraPos - vertexWorldPos.xyz);
    vec3 lightReflect = normalize(reflect(lightDir, sampledNormal));

	//Lambert
	float observedArea = max(dot(sampledNormal,-lightDir),0.f);
	float diffuseReflection = 1.f;
	vec3  lambert = albedo * diffuseReflection / PI;

	// Specular
	float alpha = max(dot(lightReflect,viewDir),0.f);
	float shininess = 25.f;
	float specularReflection = pow(alpha,shininess*glossiness) * specular.r;
	vec3 finalColor =  clamp((7* lambert.rgb +specularReflection) * (observedArea + AmbientLight),0,1);


	switch (fragRenderMode)
	{
		case 0:
			return vec4(finalColor,1.f);
		break;
		case 1:
			return vec4(albedo,1.f);
			break;
		case 2:
			return vec4(sampledNormal,1.f);
			break;
		case 3:
			return vec4(vec3(glossiness),1.f);
			break;
		case 4:
			return vec4(vec3(specular),1.f);
		break;
		default:
			return vec4(vec3(0.f),1.f);
		break;

	}

}


void main()
{
	//outColor = texture(normalSampler, fragUV);
	outColor = ShadePixel(fragNormal,fragUV,fragTangent,fragCameraPosition,fragVertexPosition);
}
