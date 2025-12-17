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

    faces.clear();
    edges.clear();

    //bottom face
    faces.push_back({0, 3, 2, 1});

    //top face
    faces.push_back({4, 5, 6, 7});

    //side faces
    faces.push_back({0, 1, 5, 4}); 
    faces.push_back({1, 2, 6, 5}); 
    faces.push_back({2, 3, 7, 6}); 
    faces.push_back({3, 0, 4, 7});

    // Base Edges
    edges.push_back({0, 1}); edges.push_back({1, 2}); 
    edges.push_back({2, 3}); edges.push_back({3, 0});
    
    // Top Edges
    edges.push_back({4, 5}); edges.push_back({5, 6}); 
    edges.push_back({6, 7}); edges.push_back({7, 4});
    
    // Vertical Edges
    edges.push_back({0, 4}); edges.push_back({1, 5}); 
    edges.push_back({2, 6}); edges.push_back({3, 7});
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

    faces.clear();
    edges.clear();

    //bottom face
    faces.push_back({0, 3, 2, 1});

    //top face
    faces.push_back({4, 5, 6, 7});

    //side faces
    faces.push_back({0, 1, 5, 4}); 
    faces.push_back({1, 2, 6, 5}); 
    faces.push_back({2, 3, 7, 6}); 
    faces.push_back({3, 0, 4, 7});

    // Base Edges
    edges.push_back({0, 1}); edges.push_back({1, 2}); 
    edges.push_back({2, 3}); edges.push_back({3, 0});
    
    // Top Edges
    edges.push_back({4, 5}); edges.push_back({5, 6}); 
    edges.push_back({6, 7}); edges.push_back({7, 4});
    
    // Vertical Edges
    edges.push_back({0, 4}); edges.push_back({1, 5}); 
    edges.push_back({2, 6}); edges.push_back({3, 7});
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

    faces.clear();
    edges.clear();

    const int APEX_INDEX = 4;

    faces.push_back({0, 3, 2, 1}); 

    faces.push_back({0, 1, APEX_INDEX}); 
    
    faces.push_back({1, 2, APEX_INDEX}); 
    
    faces.push_back({2, 3, APEX_INDEX}); 
    
    faces.push_back({3, 0, APEX_INDEX});

    // Base Edges (4)
    edges.push_back({0, 1}); 
    edges.push_back({1, 2}); 
    edges.push_back({2, 3}); 
    edges.push_back({3, 0});
    
    // Edges to Apex (4)
    edges.push_back({0, APEX_INDEX}); 
    edges.push_back({1, APEX_INDEX}); 
    edges.push_back({2, APEX_INDEX}); 
    edges.push_back({3, APEX_INDEX});
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
