#pragma once
#include "collider.h"
#include <vector>
#include "raylib.h"

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

CollisionResult SAT3DPrism(const Vector3& positionA, const vector<Vector3>& verticesA, const Vector3& positionB, const vector<Vector3>& verticesB);

Projection ProjectVertices3D(const vector<Vector3>& vertices, const Vector3& axis);

RayCollision PrsimRayHit(Ray raycast, const vector<Vector3>& vertices);

Vector3 ClosestPointOnPoly(const vector<Vector3>& verts, const Vector3& center);

CollisionResult PrismVsSphere(const Vector3& positionA, const vector<Vector3>& verticesA, const Vector3& centerB, float radius);