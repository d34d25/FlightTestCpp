#include "drawing.h"
#include<iostream>
#include <simpleTransform.h>

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


void DrawFlatShadedModel(const Transform& transform, const Model& model,
FlatShaderData* shaderData)
{
    rlPushMatrix();
    const float* matrix = GetLocalMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    rlScalef(transform.scale.x, transform.scale.y, transform.scale.z);
    DrawShadedMesh(shaderData, model);
    rlPopMatrix();
}

void DrawCollider(const std::vector<Vector3>& transformedVertices, Color color)
{
    switch (transformedVertices.size())
    {
    case 8:
        DrawLine3D(transformedVertices[0], transformedVertices[1], color);
        DrawLine3D(transformedVertices[1], transformedVertices[2], color);
        DrawLine3D(transformedVertices[2], transformedVertices[3], color);
        DrawLine3D(transformedVertices[3], transformedVertices[0], color);

        DrawLine3D(transformedVertices[4], transformedVertices[5], color);
        DrawLine3D(transformedVertices[5], transformedVertices[6], color);
        DrawLine3D(transformedVertices[6], transformedVertices[7], color);
        DrawLine3D(transformedVertices[7], transformedVertices[4], color);

        DrawLine3D(transformedVertices[0], transformedVertices[4], color);
        DrawLine3D(transformedVertices[1], transformedVertices[5], color);
        DrawLine3D(transformedVertices[2], transformedVertices[6], color);
        DrawLine3D(transformedVertices[3], transformedVertices[7], color);
        break;
    case 5:
        DrawLine3D(transformedVertices[0], transformedVertices[1], color);
        DrawLine3D(transformedVertices[1], transformedVertices[2], color);
        DrawLine3D(transformedVertices[2], transformedVertices[3], color);
        DrawLine3D(transformedVertices[3], transformedVertices[0], color);

        DrawLine3D(transformedVertices[0], transformedVertices[4], color);
        DrawLine3D(transformedVertices[1], transformedVertices[4], color);
        DrawLine3D(transformedVertices[2], transformedVertices[4], color);
        DrawLine3D(transformedVertices[3], transformedVertices[4], color);
        break;

    default:
        return;
        break;
    }

    
}

void DrawBullet(const Transform &transform)
{
    float scale = 4.0f;
    rlPushMatrix();
    const float* matrix = GetLocalMatrixTransform(transform).data();
    rlMultMatrixf(matrix);
    rlScalef(scale,scale,scale);
    DrawCube({0,0,0},0.2,0.2,1,YELLOW);
    rlPopMatrix();
}
