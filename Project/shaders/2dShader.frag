#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in float deltaTime;

layout (location = 0) out vec4 outColor;
layout(push_constant) uniform Push 
{
	mat4 transform;
	mat4 normalMatrix;
} push;
void main()
{
	 // Calculate the rainbow color using the sine function
    float r = 0.5 + 0.5 * cos(deltaTime + 0.0);
    float g = 0.5 + 0.5 * cos(deltaTime + 2.0);
    float b = 0.5 + 0.5 * cos(deltaTime + 4.0);
    
    // Combine the dynamic rainbow color with the fragment color
    vec3 rainbowColor = vec3(r, g, b) * fragColor;

    // Output the color
    outColor = vec4(rainbowColor, 1.0);

}