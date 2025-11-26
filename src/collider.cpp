#include "collider.h"
#include "raymath.h"

void Collider::CreatePrismatoidUp(float baseWidth, float baseLenght, float topWidth, float topLength, float height)
{
    float halfBW = baseWidth * 0.5f;
    float halfBL = baseLenght * 0.5f;
    float halfTW = topWidth * 0.5f;
    float halfTL = topLength * 0.5f;
    float halfH = height * 0.5f;

    localVertices =
    {
        // Bottom face (y = -halfH)
        { -halfBW, -halfH, -halfBL }, // back-left
        {  halfBW, -halfH, -halfBL }, // back-right
        {  halfBW, -halfH,  halfBL }, // front-right
        { -halfBW, -halfH,  halfBL }, // front-left

        // Top face (y = +halfH)
        { -halfTW,  halfH, -halfTL }, // back-left
        {  halfTW,  halfH, -halfTL }, // back-right
        {  halfTW,  halfH,  halfTL }, // front-right
        { -halfTW,  halfH,  halfTL }  // front-left
    };
}

void Collider::CreatePrismatoidForward(float backWidth, float backHeight, float frontWidth, float frontHeight, float length)
{
    float halfBW = backWidth * 0.5f;
    float halfBH = backHeight * 0.5f;
    float halfFW = frontWidth * 0.5f;
    float halfFH = frontHeight * 0.5f;
    float halfL  = length * 0.5f;

    localVertices =
    {
        // Back face (z = -halfL)
        { -halfBW, -halfBH, -halfL }, // bottom-left
        {  halfBW, -halfBH, -halfL }, // bottom-right
        {  halfBW,  halfBH, -halfL }, // top-right
        { -halfBW,  halfBH, -halfL }, // top-left

        // Front face (z = +halfL)
        { -halfFW, -halfFH,  halfL }, // bottom-left
        {  halfFW, -halfFH,  halfL }, // bottom-right
        {  halfFW,  halfFH,  halfL }, // top-right
        { -halfFW,  halfFH,  halfL }  // top-left
    };
}

void Collider::CreatePyramid(float baseWidth, float baseLenght, float height)
{
    float halfBW = baseWidth * 0.5f;
    float halfBL = baseLenght * 0.5f;
    float halfH = height * 0.5f;

    localVertices = 
    {
        // Base rectangle (y = -halfH)
        { -halfBW, -halfH, -halfBL }, // back-left
        {  halfBW, -halfH, -halfBL }, // back-right
        {  halfBW, -halfH,  halfBL }, // front-right
        { -halfBW, -halfH,  halfBL }, // front-left

        // Apex (y = +halfH)
        { 0, halfH, 0 }
    };
}

std::vector<Vector3> Collider::GetTransformedVertices(const Transform& transform)
{
    std::vector<Vector3> transformed;

    Matrix rot = QuaternionToMatrix(transform.rotation);

    Matrix translation = MatrixTranslate(
        transform.translation.x, transform.translation.y, transform.translation.z
    );

    Matrix t = MatrixMultiply(rot,translation);

    for (int i = 0; i < localVertices.size(); i++)
    {
        transformed.push_back(Vector3Transform(localVertices[i], t));
    }

    return transformed;
}
