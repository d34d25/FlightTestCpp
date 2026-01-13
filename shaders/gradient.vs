#version 330

in vec3 vertexPos;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;

out vec3 fragPosition;
out vec2 fragTexCoord;

void main()
{
    fragPosition = vertexPos;
    fragTexCoord = vertexTexCoord;

    gl_Position = mvp * vec4(vertexPos, 1.0);
}