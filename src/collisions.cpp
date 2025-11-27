#include "collisions.h"
#include <cmath>
#include "raymath.h"

CollisionResult SAT3D(const Vector3& positionA, const vector<Vector3>& verticesA, const Vector3& positionB, const vector<Vector3>& verticesB)
{
    CollisionResult result = {};

    float depth = INFINITY;
    Vector3 normal = {0.0f,0.0f,0.0f};

    vector<Vector3> axes;

    switch (verticesA.size())
    {
    case 8: //prismatoid

        //top
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesA[1] - verticesA[0],
            verticesA[2] - verticesA[0])
        ));

        //bottom
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesA[5] - verticesA[4], verticesA[6] - verticesA[4])
        ));

        //sides
        for (int i = 0; i < 4; ++i)
        {
            int next = (i+1)%4;
            Vector3 edge1 = verticesA[4 + i] - verticesA[i];
            Vector3 edge2 = verticesA[4 + next] - verticesA[i];
            axes.push_back(Vector3Normalize(
                Vector3CrossProduct(edge1,edge2)
            ));
        }
        break;
    
    case 5: //pyramid
        
        //base
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesA[1] - verticesA[0], verticesA[2] - verticesA[0])
        ));

        for (int i = 0; i < 4; ++i) //sides
        {
            int next = (i+1)%4;
            Vector3 edge1 = verticesA[4] - verticesA[i];
            Vector3 edge2 = verticesA[next] - verticesA[i];
            axes.push_back(Vector3Normalize(
                Vector3CrossProduct(edge1,edge2)
            ));
        }

        break;

    default:

        result.collision = false;

        return result;
        break;
    }

    switch (verticesB.size())
    {
    case 8: //prismatoid

        //top
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesB[1] - verticesB[0],
            verticesB[2] - verticesB[0])
        ));

        //bottom
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesB[5] - verticesB[4], verticesB[6] - verticesB[4])
        ));

        //sides
        for (int i = 0; i < 4; ++i)
        {
            int next = (i+1)%4;
            Vector3 edge1 = verticesB[4 + i] - verticesB[i];
            Vector3 edge2 = verticesB[4 + next] - verticesB[i];
            axes.push_back(Vector3Normalize(
                Vector3CrossProduct(edge1,edge2)
            ));
        }
        break;
    
    case 5: //pyramid
        
        //base
        axes.push_back(Vector3Normalize(
            Vector3CrossProduct(verticesB[1] - verticesB[0], verticesB[2] - verticesB[0])
        ));

        for (int i = 0; i < 4; ++i) //sides
        {
            int next = (i+1)%4;
            Vector3 edge1 = verticesB[4] - verticesB[i];
            Vector3 edge2 = verticesB[next] - verticesB[i];
            axes.push_back(Vector3Normalize(
                Vector3CrossProduct(edge1,edge2)
            ));
        }

        break;

    default:

        result.collision = false;

        return result;
        break;
    }

    for (int i = 0; i < verticesA.size(); i++)
    {
        Vector3 edgeA = verticesA[(i+1) % verticesA.size()] - verticesA[i];

        for (int j = 0; j < verticesB.size(); j++)
        {
            Vector3 edgeB = verticesB[(j+1)%verticesB.size()] - verticesB[j];
            Vector3 axis = Vector3CrossProduct(edgeA,edgeB);
            if(Vector3Length(axis) > 1e-6f)
            {
                axes.push_back(Vector3Normalize(axis));
            }
        }
    }

    for (int i = 0; i < axes.size(); i++)
    {
        Projection projA = ProjectVertices3D(verticesA, axes[i]);
        Projection projB = ProjectVertices3D(verticesB, axes[i]);

        if (projA.max < projB.min || projB.max < projA.min)
        {
            result.collision = false;
            return result;
        }

        float axisDepth = min(projA.max - projB.min, projB.max - projA.min);

        if (axisDepth < depth)
        {
            depth = axisDepth;
            normal = axes[i];
        }
    }

    Vector3 direction = Vector3Subtract(positionB, positionA);

    if (Vector3DotProduct(direction, normal) < 0.0f)
    {
        normal.x = -normal.x;
        normal.y = -normal.y;
        normal.z = -normal.z;
    }

    result.collision = true;
    result.normal = normal;
    result.depth = depth;

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


