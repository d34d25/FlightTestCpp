#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include "simpleTransform.h"
#include "target.h"
#include <iostream>

#define BULLET_YELLOW Color{255,255,100,255}

struct FlatShaderData
{
    float minIntensity = 0.4f;
    float maxIntensity = 1.0f;

    float skipIntensity;

    Vector3 lightDir = {0.5f,0.75f,0.0f};

    Shader __shader;
    int __lightDirLoc;
    int __baseColorLoc;
    int __minIntensityLoc;
    int __maxIntensityLoc;
    int __skipMaterial;

    Vector4 __lastBaseColor = {-100,-100,-100,-100};
};

FlatShaderData mLoadFlatShader(const char* shaderVs, const char* shaderFs, int skipIndex);

void mApplyFlatShader(FlatShaderData* shaderData, Model* model);

void DrawShadedMesh(FlatShaderData* shaderData, Model model);

inline void DrawFlatShadedModel(const Transform& transform, const Model& model, FlatShaderData* shaderData)
{
    rlPushMatrix();
    const float* matrix = GetWorldMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    rlScalef(transform.scale.x, transform.scale.y, transform.scale.z);
    DrawShadedMesh(shaderData, model);
    rlPopMatrix();
}

inline void DrawColliderWire(Collider& collider, const Transform& transform, Color color)
{
    std::vector<Vector3> vertices = collider.GetTransformedVertices(transform);

    for(int i = 0; i < collider.edges.size(); i++)
    {
        const std::pair<int,int>& edge = collider.edges[i];

        Vector3 startPos = vertices[edge.first];
        Vector3 endPos = vertices[edge.second];

        DrawLine3D(startPos, endPos, color);
    }
}

inline void DrawCollider(Collider& collider, const Transform& transform, Color color)
{
    std::vector<Vector3> vertices = collider.GetTransformedVertices(transform);

    for (int i = 0; i < collider.faces.size(); i++)
    {
        const std::vector<int>& face_indices = collider.faces[i];

        if (face_indices.size() < 3) continue;

        Vector3 v0 = vertices[face_indices[0]];

        for(int j = 0; j < face_indices.size(); j++)
        {
            Vector3 v1 = vertices[face_indices[j]];
            Vector3 v2 = vertices[face_indices[(j + 1) % face_indices.size()]];

            DrawTriangle3D(v0,v1,v2, color);
        }         
    }
}

inline Color GetNormalColor(const Vector3& normal)
{
    Vector3 normalN = Vector3Normalize(normal);

    float ax = fabsf(normalN.x);
    float ay = fabsf(normalN.y);
    float az = fabsf(normalN.z);

    if (ax > ay && ax > az) return RED;
    if (ay > ax && ay > az) return GREEN;
    return BLUE;           
}

inline void DrawColliderFaceNormals(Collider& collider, const Transform& transform, float size)
{
    std::vector<Vector3> vertices = collider.GetTransformedVertices(transform);

    for (int i = 0; i < collider.faces.size(); i++)
    {
        const std::vector<int>& face_indices = collider.faces[i];

        if (face_indices.size() < 3) continue;

        Vector3 v0 = vertices[face_indices[0]]; 
        Vector3 v1 = vertices[face_indices[1]]; 
        Vector3 v2 = vertices[face_indices[2]];

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;

        Vector3 normal = Vector3CrossProduct(edge1, edge2);
        normal = Vector3Normalize(normal);

        Vector3 center = {0,0,0};

        for(int j = 0; j < face_indices.size(); j++)
        {
            center = Vector3Add(center, vertices[face_indices[j]]);
        }

        center = Vector3Scale(center, 1.0f / face_indices.size());

        Vector3 lv0 = collider.localVertices[face_indices[0]];
        Vector3 lv1 = collider.localVertices[face_indices[1]];
        Vector3 lv2 = collider.localVertices[face_indices[2]];

        Vector3 localNormal = Vector3Normalize(
            Vector3CrossProduct(lv1 - lv0, lv2 - lv0)
        );

        Color normalColor = GetNormalColor(localNormal);
        
        DrawLine3D(center, Vector3Add(center, Vector3Scale(normal, size)), normalColor);

        DrawSphere(center, 0.05f, YELLOW);
    }
}

inline void DrawBullet(const Transform& transform, float radius, Color color)
{
    rlPushMatrix();
    const float* matrix = GetWorldMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    DrawCube({0,0,0},0.5f,0.5f,radius * 2.0f,color);
    rlPopMatrix();
}

inline void DrawMissile(const Transform& transform, float radius)
{
    rlPushMatrix();
    const float* matrix = GetWorldMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    DrawCube({0,0,0},2.0f,2.0f,radius * 2.0f,{200,200,200,255});
    rlPopMatrix();
}

inline void DrawCircleRotated3D(const Transform& transform, float radius, Color color)
{
    rlPushMatrix();
    const float* matrix = GetWorldMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    DrawCircleSector({0,0}, radius, 0, 360, 10, color);
    rlPopMatrix();
}

inline void DrawTgt(const Target& tgt)
{
    rlPushMatrix();
    const float* matrix = GetWorldMatrixTransform(tgt.body.transform).data();
    rlMultMatrixf(matrix);
    DrawCubeWires({0,0,0},tgt.width, tgt.height, tgt.length, GREEN);
    rlPopMatrix();
}


inline Model GenerateGroundMesh(Texture2D texture, float width, float height, int textureRepeatCount)
{
    Mesh mesh = GenMeshPlane(width, height,1,1);

    for (int i = 0; i < mesh.vertexCount * 2; i++)
    {
        mesh.texcoords[i] *= textureRepeatCount;
    }

    UpdateMeshBuffer(mesh, 1, mesh.texcoords, mesh.vertexCount * 2 * sizeof(float), 0);

    Model model = LoadModelFromMesh(mesh);

    SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texture;
    
    return model;
}


inline void DrawGround(Model model)
{
    DrawModel(model, {0,0,0},1,WHITE);
}

//DrawCircleSector({0,0}, radius, 0, PI, 10, ORANGE); this looks like an arrow, cool for using it in the HUD