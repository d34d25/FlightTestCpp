#include "drawing.h"
#include<iostream>


FlatShaderData mLoadFlatShader(const char* shaderVs, const char* shaderFs, int skipIndex)
{
    FlatShaderData shaderData;

    shaderData.__shader = LoadShader(shaderVs, shaderFs);

    shaderData.__lightDirLoc = GetShaderLocation(shaderData.__shader, "lightDir");
    shaderData.__baseColorLoc = GetShaderLocation(shaderData.__shader, "baseColor");
    shaderData.__minIntensityLoc = GetShaderLocation(shaderData.__shader,"minIntensity");
    shaderData.__maxIntensityLoc = GetShaderLocation(shaderData.__shader, "maxIntensity");
    
    shaderData.__skipMaterial = skipIndex;

    std::cout<<"shader loaded correclty" <<"\n";

    return shaderData;
}

GradientShaderData mLoadGradientShader(const char* shaderVs, const char* shaderFs, Color baseColor, Color topColor, Color bottomColor, float minHeigth, float maxHeight)
{
    GradientShaderData shaderData;

    shaderData.baseColor = baseColor;
    shaderData.topColor = topColor;
    shaderData.bottomColor = bottomColor;

    shaderData.__shader = LoadShader(shaderVs, shaderFs);

    shaderData.baseColorLoc = GetShaderLocation(shaderData.__shader, "baseColor");
    shaderData.topColorLoc = GetShaderLocation(shaderData.__shader, "topColor");
    shaderData.bottomColorLoc = GetShaderLocation(shaderData.__shader, "bottomColor");

    shaderData.minHeightLoc = GetShaderLocation(shaderData.__shader, "minHeight");
    shaderData.maxHeightLoc = GetShaderLocation(shaderData.__shader, "maxHeight");

    shaderData.minHeight = minHeigth;
    shaderData.maxHeight = maxHeight;

    return shaderData;
}

void mApplyFlatShader(FlatShaderData* shaderData, Model* model)
{
    for (int i = 0; i < model->materialCount; i++)
    {
        model->materials[i].shader = shaderData->__shader; 
    }

    float lightDirArray[3];
    lightDirArray[0] = shaderData->lightDir.x;
    lightDirArray[1] = shaderData->lightDir.y;
    lightDirArray[2] = shaderData->lightDir.z;

    float f = 1.0f;

    SetShaderValue(shaderData->__shader, 
        shaderData->__lightDirLoc, lightDirArray, SHADER_UNIFORM_VEC3);

    SetShaderValue(shaderData->__shader,
    shaderData->__lightDirLoc,&f, SHADER_UNIFORM_FLOAT);

    SetShaderValue(shaderData->__shader,
    shaderData->__minIntensityLoc, &shaderData->minIntensity,
    SHADER_UNIFORM_FLOAT);

    SetShaderValue(shaderData->__shader,
    shaderData->__maxIntensityLoc, &shaderData->maxIntensity,
    SHADER_UNIFORM_FLOAT);

    std::cout<<"shader applied correclty" <<"\n";
}

void mApplyGradientShader(GradientShaderData *shaderData, Model *model)
{
    for (int i = 0; i < model->materialCount; i++)
    {
        model->materials[i].shader = shaderData->__shader; 
    }

    float colorArray_Base[4];
    float colorArray_Top[4];
    float colorArray_Bottom[4];

    colorArray_Base[0] = shaderData->baseColor.r / 255.0f;
    colorArray_Base[1] = shaderData->baseColor.g / 255.0f;
    colorArray_Base[2] = shaderData->baseColor.b / 255.0f;
    colorArray_Base[3] = shaderData->baseColor.a / 255.0f;

    colorArray_Top[0] = shaderData->topColor.r / 255.0f;
    colorArray_Top[1] = shaderData->topColor.g / 255.0f;
    colorArray_Top[2] = shaderData->topColor.b / 255.0f;
    colorArray_Top[3] = shaderData->topColor.a / 255.0f;

    colorArray_Bottom[0] = shaderData->bottomColor.r / 255.0f;
    colorArray_Bottom[1] = shaderData->bottomColor.g / 255.0f;
    colorArray_Bottom[2] = shaderData->bottomColor.b / 255.0f;
    colorArray_Bottom[3] = shaderData->bottomColor.a / 255.0f;

    SetShaderValue(shaderData->__shader, 
    shaderData->baseColorLoc, colorArray_Base, SHADER_UNIFORM_VEC4);
    
    SetShaderValue(shaderData->__shader, 
    shaderData->topColorLoc, colorArray_Top, SHADER_UNIFORM_VEC4);

    SetShaderValue(shaderData->__shader, 
    shaderData->bottomColorLoc, colorArray_Bottom, SHADER_UNIFORM_VEC4);

    SetShaderValue(shaderData->__shader,
    shaderData->minHeightLoc, &shaderData->minHeight, SHADER_UNIFORM_FLOAT); //update this

    SetShaderValue(shaderData->__shader,
    shaderData->maxHeightLoc, &shaderData->maxHeight, SHADER_UNIFORM_FLOAT);
}

void DrawShadedMesh(FlatShaderData* shaderData, Model model)
{
    float baseColorArray[4];

    for (int i = 0; i < model.meshCount; i++)
    {
        int matIndex = model.meshMaterial[i];

        Color baseColor = model.materials[matIndex].maps[0].color;

        baseColorArray[0] = baseColor.r / 255.0f;
        baseColorArray[1] = baseColor.g / 255.0f;
        baseColorArray[2] = baseColor.b / 255.0f;

        if(matIndex == shaderData->__skipMaterial)
        {
            baseColorArray[3] = -shaderData->skipIntensity;
        }
        else
        {
            baseColorArray[3] = baseColor.a / 255.0f;
        }

        if (baseColorArray[0] != shaderData->__lastBaseColor.x|| baseColorArray[1] != shaderData->__lastBaseColor.y || 
            baseColorArray[2] != shaderData->__lastBaseColor.z || baseColorArray[3] != shaderData->__lastBaseColor.w)
        {
            SetShaderValue(shaderData->__shader,
            shaderData->__baseColorLoc, baseColorArray, SHADER_UNIFORM_VEC4);

            shaderData->__lastBaseColor = {
                baseColorArray[0], baseColorArray[1], baseColorArray[2],
                baseColorArray[3]
            };
        }

        DrawMesh(model.meshes[i], model.materials[matIndex], MatrixIdentity());
    }
}

