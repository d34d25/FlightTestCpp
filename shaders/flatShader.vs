#version 330

in vec3 vertexPos;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragNormal;

void main()
{
    gl_Position = mvp * vec4(vertexPos, 1.0);
    fragNormal = mat3(matModel) * vertexNormal;
}