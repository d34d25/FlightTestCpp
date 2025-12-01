#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include "simpleTransform.h"

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
    const float* matrix = GetLocalMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    rlScalef(transform.scale.x, transform.scale.y, transform.scale.z);
    DrawShadedMesh(shaderData, model);
    rlPopMatrix();
}

void DrawColliderWire(const std::vector<Vector3>& transformedVertices, Color color);

void DrawCollider(const std::vector<Vector3>& v, Color color);

inline void DrawBullet(const Transform& transform)
{
    rlPushMatrix();
    const float* matrix = GetLocalMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    DrawCube({0,0,0},0.5f,0.5f,4.0f,{255,255,100,255});
    rlPopMatrix();
}


inline void DrawMissile(const Transform& transform)
{
    rlPushMatrix();
    const float* matrix = GetLocalMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    DrawCube({0,0,0},1.5f,1.5f,4.0f,{200,200,200,255});
    rlPopMatrix();
}