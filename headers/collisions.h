#pragma once
#include "collider.h"
#include <vector>

struct Projection
{
    float min, max;
};

struct CollisionResult
{
    bool collision;
    float depth;
    Vector3 normal;
};

using namespace std;

CollisionResult SAT3D(const Vector3& positionA, const vector<Vector3>& verticesA, const Vector3& positionB, const vector<Vector3>& verticesB);

Projection ProjectVertices3D(const vector<Vector3>& vertices, const Vector3& axis);