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

void DrawColliderWire(const std::vector<Vector3>& transformedVertices, Color color);

void DrawCollider(const std::vector<Vector3>& v, Color color);

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