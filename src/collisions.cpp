#include "collisions.h"
#include <cmath>
#include "raymath.h"
#include "raylib.h"
#include <iostream>

const float MY_EPSILON = 1e-5f;
const float MY_EPSILON_SQ = 1e-10f;

//helpers
Vector3 ClosestPointOnPoly(const std::vector<Vector3>& verts, const std::vector<std::vector<int>>& faces,const std::vector<std::pair<int, int>>& edges,const Vector3& center)
{
    float bestDistSq = INFINITY;
    Vector3 closestPoint = verts[0];

    for (int i = 0; i < verts.size(); i++)
    {
        float d = Vector3LengthSqr(Vector3Subtract(verts[i], center));

        if(d < bestDistSq - MY_EPSILON)
        {
            bestDistSq = d;
            closestPoint = verts[i];
        }
    }

    for(int i = 0; i < edges.size(); i++)
    {
        const std::pair<int, int>& edge = edges[i];

        Vector3 va = verts[edge.first];
        Vector3 vb = verts[edge.second];

        Vector3 pointOnEdge = ClosestPointOnSegment(va,vb,center);

        float d = Vector3LengthSqr(Vector3Subtract(pointOnEdge, center));

        if(d < bestDistSq - MY_EPSILON)
        {
            bestDistSq = d;
            closestPoint = pointOnEdge;
        }
    }

    for(int i = 0; i < faces.size(); i++)
    {
        const std::vector<int>& face = faces[i];

        if(face.size() < 3) continue;

        Vector3 va = verts[face[0]];

        for(int j = 1; j < face.size() - 1; j++)
        {
            Vector3 vb = verts[face[j]];
            Vector3 vc = verts[face[j+1]];

            Vector3 pointOnFace = ClosestPointOnTriangle(va,vb,vc,center);

            float d = Vector3LengthSqr(Vector3Subtract(pointOnFace, center));

            if(d < bestDistSq - MY_EPSILON)
            {
                bestDistSq = d;
                closestPoint = pointOnFace;
            }
        }
    }

    return closestPoint;
}

Vector3 ClosestPointOnSegment(const Vector3 &va, const Vector3 &vb, const Vector3 &center)
{
    Vector3 vectorAB = Vector3Subtract(vb, va);
    Vector3 vectorAP = Vector3Subtract(center, va);

    float lenSqAB = Vector3LengthSqr(vectorAB);

    if (lenSqAB == 0.0f) return va;

    float t = Vector3DotProduct(vectorAP, vectorAB) / lenSqAB;

    if (t < 0.0f) t = 0.0f;

    else if (t > 1.0f) t = 1.0f;
   
    Vector3 scaledAB = Vector3Scale(vectorAB, t);
    Vector3 closestPoint = Vector3Add(va, scaledAB);

    return closestPoint;
}

Vector3 ClosestPointOnTriangle(const Vector3 &va, const Vector3 &vb, const Vector3 &vc, const Vector3 &center)
{
    Vector3 vectorAB = Vector3Subtract(vb, va);
    Vector3 vectorAC = Vector3Subtract(vc, va);
    Vector3 vectorAP = Vector3Subtract(center, va);

    float d1 = Vector3DotProduct(vectorAB, vectorAP);
    float d2 = Vector3DotProduct(vectorAC, vectorAP);

    if (d1 <= 0.0f && d2 <= 0.0f) return va;

    Vector3 vectorBP = Vector3Subtract(center, vb);
    float d3 = Vector3DotProduct(vectorAB, vectorBP);
    float d4 = Vector3DotProduct(vectorAC, vectorBP);

    if (d3 >= 0.0f && d4 <= d3) return vb;

    if (d1 >= 0.0f && d3 <= 0.0f)
    {
        float denom = Vector3LengthSqr(vectorAB);
        float t = d1 / denom;

        return ClosestPointOnSegment(va, vb, center);
    }

    Vector3 vectorCP = Vector3Subtract(center, vc);
    float d5 = Vector3DotProduct(vectorAB, vectorCP);
    float d6 = Vector3DotProduct(vectorAC, vectorCP);

    if (d6 >= 0.0f && d5 <= d6) return vc;

    if (d2 >= 0.0f && d6 <= 0.0f) return ClosestPointOnSegment(va, vc, center);

    Vector3 vectorCB = Vector3Subtract(vb, vc);
    Vector3 vectorCA = Vector3Subtract(va, vc);

    float vc_d3 = Vector3DotProduct(vectorCB, vectorCP);
    float vc_d4 = Vector3DotProduct(vectorCA, vectorCP);

    if (vc_d3 >= 0.0f && vc_d4 <= vc_d3) return ClosestPointOnSegment(vb, vc, center);

    float d11 = Vector3DotProduct(vectorAB, vectorAB);
    float d22 = Vector3DotProduct(vectorAC, vectorAC);
    float d12 = Vector3DotProduct(vectorAB, vectorAC);

    float denom = d11 * d22 - d12 * d12;
    
    if (fabs(denom) < MY_EPSILON) return ClosestPointOnSegment(va, vb, center);

    float invDenom = 1.0f / denom;

    float v = (d22 * d1 - d12 * d2) * invDenom;
    float w = (d11 * d2 - d12 * d1) * invDenom;

    v = Clamp(v, 0.0f, 1.0f);
    w = Clamp(w, 0.0f, 1.0f - v);

    Vector3 result = Vector3Add(va, Vector3Scale(vectorAB, v));
    result = Vector3Add(result, Vector3Scale(vectorAC, w));

    return result;
}

Projection ProjectVertices3D(const vector<Vector3> &vertices, const Vector3 &axis)
{
    float min = INFINITY;
    float max = -INFINITY;

    for (int i = 0; i < vertices.size(); i++)
    {
        float projection = Vector3DotProduct(vertices[i], axis);
        if (projection < min) min = projection;
        if (projection > max) max = projection;
    }

    return {min, max};
}

//continious collision detection
CollisionResult_CCD SAT3DPoly_CCD(Collider& colliderA, const Transform& transformA, Collider& colliderB, const Transform& transformB, const Vector3& relVel)
{
    CollisionResult_CCD result = {};

    vector<Vector3> axes;

    std::vector<Vector3> verticesA = colliderA.GetTransformedVertices(transformA);
    std::vector<Vector3> verticesB = colliderB.GetTransformedVertices(transformB);

    //faces
    for(int i = 0; i < colliderA.faces.size(); i++)
    {
        const std::vector<int>& face_indices = colliderA.faces[i];

        if (face_indices.size() < 3) continue;

        Vector3 v0 = verticesA[face_indices[0]]; 
        Vector3 v1 = verticesA[face_indices[1]]; 
        Vector3 v2 = verticesA[face_indices[2]];

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;

        Vector3 normal = Vector3CrossProduct(edge1, edge2);

        if(Vector3LengthSqr(normal) > MY_EPSILON_SQ)
        {
            axes.push_back(Vector3Normalize(normal));
        }
    }

    for(int i = 0; i < colliderB.faces.size(); i++)
    {
        const std::vector<int>& face_indices = colliderB.faces[i];

        if (face_indices.size() < 3) continue;

        Vector3 v0 = verticesB[face_indices[0]]; 
        Vector3 v1 = verticesB[face_indices[1]]; 
        Vector3 v2 = verticesB[face_indices[2]];

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;

        Vector3 normal = Vector3CrossProduct(edge1, edge2);

        if(Vector3LengthSqr(normal) > MY_EPSILON_SQ)
        {
            axes.push_back(Vector3Normalize(normal));
        }
    }

    //edges
    for(int i = 0; i < colliderA.edges.size(); i++)
    {
        const std::pair<int, int>& edgeA_indices = colliderA.edges[i];

        Vector3 eA_P1 = verticesA[edgeA_indices.first];
        Vector3 eA_P2 = verticesA[edgeA_indices.second];
        Vector3 edgeA_Vector = eA_P2 - eA_P1;

        for(int j = 0; j < colliderB.edges.size(); j++)
        {
            const std::pair<int, int>& edgeB_indices = colliderB.edges[j];
            
            Vector3 eB_P1 = verticesB[edgeB_indices.first];
            Vector3 eB_P2 = verticesB[edgeB_indices.second];
            Vector3 edgeB_Vector = eB_P2 - eB_P1;
            
            Vector3 axis = Vector3CrossProduct(edgeA_Vector, edgeB_Vector);
            
            if(Vector3LengthSqr(axis) > MY_EPSILON_SQ)
            {
                axes.push_back(Vector3Normalize(axis));
            }
        }
    }

    if(Vector3LengthSqr(relVel) > MY_EPSILON_SQ)
    {
        for(int i = 0; i < colliderA.edges.size(); i++)
        {
            const std::pair<int, int>& edgeA_indices = colliderA.edges[i];

            Vector3 edge = Vector3Subtract(
                verticesA[edgeA_indices.first],
                verticesA[edgeA_indices.second]
            );

            Vector3 axis = Vector3CrossProduct(edge, relVel);

            if(Vector3LengthSqr(axis) > MY_EPSILON_SQ)
            {
                axes.push_back(Vector3Normalize(axis));
            }
        }

        for(int i = 0; i < colliderB.edges.size(); i++)
        {
            const std::pair<int, int>& edgeB_indices = colliderB.edges[i];

            Vector3 edge = Vector3Subtract(
                verticesB[edgeB_indices.first],
                verticesB[edgeB_indices.second]
            );

            Vector3 axis = Vector3CrossProduct(edge, relVel);

            if(Vector3LengthSqr(axis) > MY_EPSILON_SQ)
            {
                axes.push_back(Vector3Normalize(axis));
            }
        }
    }

    for(int i = 0; i < verticesA.size(); i++)
    {
        Vector3 vertex = verticesA[i];

        for(int j = 0; j < colliderB.faces.size(); j++)
        {
            const std::vector<int>& face_indices = colliderB.faces[j];

            Vector3 v0 = verticesB[face_indices[0]]; 
            Vector3 v1 = verticesB[face_indices[1]]; 
            Vector3 v2 = verticesB[face_indices[2]];

            Vector3 edge1 = v1 - v0;
            Vector3 edge2 = v2 - v0;

            Vector3 normal = Vector3CrossProduct(edge1, edge2);

            if(Vector3LengthSqr(normal) > MY_EPSILON_SQ)
            {
                normal = Vector3Normalize(normal);

                if(Vector3DotProduct(relVel, normal) < -MY_EPSILON)
                {
                    axes.push_back(normal);
                }
            }
        }   
    }

    for(int i = 0; i < verticesB.size(); i++)
    {
        Vector3 vertex = verticesB[i];

        for(int j = 0; j < colliderA.faces.size(); j++)
        {
            const std::vector<int>& face_indices = colliderA.faces[j];

            Vector3 v0 = verticesA[face_indices[0]]; 
            Vector3 v1 = verticesA[face_indices[1]]; 
            Vector3 v2 = verticesA[face_indices[2]];

            Vector3 edge1 = v1 - v0;
            Vector3 edge2 = v2 - v0;

            Vector3 normal = Vector3CrossProduct(edge1, edge2);

            if(Vector3LengthSqr(normal) > MY_EPSILON_SQ)
            {
                normal = Vector3Normalize(normal);

                if(Vector3DotProduct(relVel, normal) < -MY_EPSILON)
                {
                    axes.push_back(normal);
                }
            }
        }   
    }

    float tStart = 0.0f;
    float tEnd = 1.0f;

    for (int i = 0; i < axes.size(); i++)
    {
        Vector3 axis = axes[i];

        Projection projA = ProjectVertices3D(verticesA, axis);
        Projection projB = ProjectVertices3D(verticesB, axis);

        float speedRel = Vector3DotProduct(relVel, axis);

        float timeIn;
        float timeOut;

        if (fabs(speedRel) > MY_EPSILON)
        {
            timeIn = (projA.min - projB.max) / speedRel;
            timeOut = (projA.max - projB.min) / speedRel;

            if(timeIn > timeOut)
            {
                std::swap(timeIn, timeOut);
            }
        }
        else
        {
            if (projA.max < projB.min || projB.max < projA.min)
            {
                result.collision = false;
                result.timeOfImpact = 1.0f;
                return result;
            }

            timeIn = 0.0f;
            timeOut = 1.0f;
        }

        if(timeIn > tEnd)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if (timeOut < tStart)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if(timeOut < 0.0f || timeIn > 1.0f)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if(timeIn > tStart)
        {
            tStart = timeIn;
        }

        if(timeOut < tEnd)
        {
            tEnd = timeOut;
        }
    }

    tStart = max(tStart, 0.0f);

    if(tStart < tEnd && tStart <= 1.0f)
    {
        result.collision = true;
        result.timeOfImpact = tStart;
    }
    else
    {
        result.collision = false;
        result.timeOfImpact = 1.0f;
    }

    return result;
}

CollisionResult_CCD PolyVsSphere_CCD(Collider& colliderA, const Transform& transformA, const Vector3& centerB, float radius, const Vector3& relVel)
{
    CollisionResult_CCD result = {};

    vector <Vector3> axes;

    std::vector<Vector3> verticesA = colliderA.GetTransformedVertices(transformA);

    //faces
    for(int i = 0; i < colliderA.faces.size(); i++)
    {
        const std::vector<int>& face_indices = colliderA.faces[i];

        if (face_indices.size() < 3) continue;

        Vector3 v0 = verticesA[face_indices[0]]; 
        Vector3 v1 = verticesA[face_indices[1]]; 
        Vector3 v2 = verticesA[face_indices[2]];

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;

        Vector3 normal = Vector3CrossProduct(edge1, edge2);

        if(Vector3LengthSqr(normal) > MY_EPSILON_SQ)
        {
            axes.push_back(Vector3Normalize(normal));
        }
    }

    Vector3 closest = ClosestPointOnPoly(
        verticesA, 
        colliderA.faces, colliderA.edges,
        centerB
    );
    
    Vector3 axisVector = Vector3Subtract(centerB, closest);

    if(Vector3LengthSqr(axisVector) > MY_EPSILON_SQ)
    {
        axes.push_back(Vector3Normalize(axisVector));
    }

    float tStart = 0.0f;
    float tEnd = 1.0f;

    for (int i = 0; i < axes.size(); i++)
    {
        Vector3 axis = axes[i];

        float minA = INFINITY, maxA = -INFINITY;

        for(int j = 0; j < verticesA.size(); j++)
        {
            Vector3 v = verticesA[j];

            float proj = Vector3DotProduct(v, axis);
            minA = min(minA, proj);
            maxA = max(maxA, proj);
        }

        float centerProj = Vector3DotProduct(centerB, axis);
        float minB = centerProj - radius;
        float maxB = centerProj + radius;

        float speedRel = Vector3DotProduct(relVel, axis);

        float timeIn;
        float timeOut;

        if (fabs(speedRel) > MY_EPSILON)
        {
            timeIn = (minA - maxB) / speedRel;
            timeOut = (maxA - minB) / speedRel;

            if(timeIn > timeOut)
            {
                std::swap(timeIn, timeOut);
            }
        }
        else
        {
            if(maxA < minB || maxB < minA)
            {
                result.collision = false;
                result.timeOfImpact = 1.0f;
                return result;
            }

            timeIn = 0.0f;
            timeOut = 1.0f;
        }

        if(timeIn > tEnd)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if (timeOut < tStart)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if(timeOut < 0.0f || timeIn > 1.0f)
        {
            result.collision = false;
            result.timeOfImpact = 1.0f;
            return result;
        }

        if(timeIn > tStart)
        {
            tStart = timeIn;
        }

        if(timeOut < tEnd)
        {
            tEnd = timeOut;
        }
    }

    tStart = max(tStart,0.0f);

    if(tStart < tEnd && tStart <= 1.0f)
    {
        result.collision = true;
        result.timeOfImpact = tStart;
    }
    else
    {
        result.collision = false;
        result.timeOfImpact = 1.0f;
    }

    return result;
}

RayCollision PrsimRayHit(Ray raycast, const vector<Vector3> &vertices)
{
    if(vertices.size() != 8)
    {
        RayCollision noHit = {};
        noHit.hit = false;
        return noHit;
    }

    RayCollision hits[6] =
    {
        GetRayCollisionQuad(raycast, vertices[0], vertices[1],vertices[2],vertices[3]),
        GetRayCollisionQuad(raycast, vertices[4], vertices[5],vertices[6],vertices[7]),

        GetRayCollisionQuad(raycast, vertices[0], vertices[1],vertices[5],vertices[4]),
        GetRayCollisionQuad(raycast, vertices[1], vertices[2],vertices[6],vertices[5]),

        GetRayCollisionQuad(raycast, vertices[2], vertices[3],vertices[7],vertices[6]),
        GetRayCollisionQuad(raycast, vertices[3], vertices[0],vertices[4],vertices[7]),
    };

    RayCollision bestHit = {};

    bestHit.hit = false;
    bestHit.distance = INFINITY;

    for (RayCollision& h : hits)
    {
        if(h.hit && h.distance < bestHit.distance)
        {
            bestHit = h;
        }
    }

    return bestHit;
}
