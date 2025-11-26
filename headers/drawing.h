#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <vector>

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

void DrawFlatShadedModel(Transform transform, Model model, FlatShaderData* shaderData);

void DrawCollider(std::vector<Vector3> transformedVertices, Color color);