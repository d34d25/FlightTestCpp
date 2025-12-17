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

struct CollisionResult_CCD
{
    bool collision;
    float timeOfImpact;
    Vector3 normal;
};

using namespace std;

//helpers

Vector3 ClosestPointOnPoly(const std::vector<Vector3>& verts, const std::vector<std::vector<int>>& faces,const std::vector<std::pair<int, int>>& edges,const Vector3& center);

Vector3 ClosestPointOnSegment(const Vector3& va, const Vector3& vb, const Vector3& center);

Vector3 ClosestPointOnTriangle(const Vector3& va, const Vector3& vb, const Vector3& vc, const Vector3& center);

Projection ProjectVertices3D(const vector<Vector3>& vertices, const Vector3& axis);

//continious collision detection

CollisionResult_CCD SAT3DPoly_CCD(Collider& colliderA, const Transform& transformA, Collider& colliderB, const Transform& transformB, const Vector3& relVel);

CollisionResult_CCD PolyVsSphere_CCD(Collider& colliderA, const Transform& transformA, const Vector3& centerB, float radius, const Vector3& relVel);

RayCollision PrsimRayHit(Ray raycast, const vector<Vector3>& vertices);