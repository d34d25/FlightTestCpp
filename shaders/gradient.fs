#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform vec4 baseColor;
uniform vec4 topColor;
uniform vec4 bottomColor;

uniform float minHeight;
uniform float maxHeight;

out vec4 finalColor;

void main()
{
    float shadeCount = 64;

    float factor = clamp((fragPosition.y - minHeight) / (maxHeight - minHeight),0.0,1.0);

    float finalFactor = floor(factor * shadeCount) / (shadeCount - 1.0);

    vec4 gradient = mix(bottomColor, topColor, finalFactor);

    finalColor = baseColor * gradient;
}