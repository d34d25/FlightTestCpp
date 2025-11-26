#pragma once
#include "raylib.h"
#include "rlgl.h"

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

FlatShaderData mLoadShader(const char* shaderVs, const char* shaderFs, int skipIndex);

void mApplyShader(FlatShaderData* shaderData, Model* model);

void DrawShadedModel(Transform transform, Model model, FlatShaderData* shaderData);