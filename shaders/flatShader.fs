#version 330

in vec3 fragNormal;

uniform vec3 lightDir;
uniform vec4 baseColor;
uniform float minIntensity;
uniform float maxIntensity;

out vec4 finalColor;

void main() 
{
    vec3 norm = normalize(fragNormal);
    vec3 light = normalize(lightDir);

    // Compute base intensity from lighting
    float intensity = max(dot(norm, light), minIntensity);
    intensity = clamp(intensity, 0.0, maxIntensity);

    if(baseColor.a < 0.0)
    {
        intensity = -baseColor.a;
    }
    else
    {
        intensity *= baseColor.a;
    }


    // Use RGB with scaled intensity, keep alpha = 1.0
    finalColor = vec4(baseColor.rgb * intensity, 1.0);
}