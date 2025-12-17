#pragma once
#include <vector>
#include "raylib.h"

class Collider
{
private:

    std::vector<Vector3> localVertices;

public:

    std::vector<std::pair<int, int>> edges;

    std::vector<std::vector<int>> faces;

    Collider() = default;

    void CreatePrismatoidUp(float baseWidth, float baseLenght, float topWidth, float topLength, float height);

    void CreatePrismatoidForward(float backWidth, float backHeight, float frontWidth, float frontHeight, float length);

    void CreatePyramid(float baseWidth, float baseLenght, float height);

    inline std::vector<Vector3> GetVertices() {return localVertices;}

    std::vector<Vector3> GetTransformedVertices(const Transform& transform);

};