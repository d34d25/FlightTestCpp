#pragma once
#include "raylib.h"
#include "raymath.h"
#include <array>

inline Matrix GetWorldTransform(const Transform& transform)
{
    Matrix rotationMatrix = QuaternionToMatrix(transform.rotation);
    Matrix translationMatrix = MatrixTranslate(
        transform.translation.x, transform.translation.y,
        transform.translation.z
    );

    return MatrixMultiply(rotationMatrix, translationMatrix);
}

inline std::array<float, 16> GetWorldMatrixTransform(const Transform& transform)
{
    Matrix matrix = GetWorldTransform(transform);

    std::array<float, 16> matrixArray = {
        matrix.m0, matrix.m1, matrix.m2, matrix.m3,
        matrix.m4, matrix.m5, matrix.m6, matrix.m7,
        matrix.m8, matrix.m9, matrix.m10, matrix.m11,
        matrix.m12, matrix.m13, matrix.m14, matrix.m15
    };

    return matrixArray;
}

inline Vector3 GetWorldForwardVector(const Transform& transform)
{
    Vector3 localForward;
    localForward.x = 0.0f, localForward.y = 0.0f, localForward.z = 1.0f;

    Vector3 forward = Vector3RotateByQuaternion(localForward, QuaternionNormalize(transform.rotation));

    forward = Vector3Normalize(forward);

    return forward;
}

inline Vector3 GetWorldUpVector(const Transform& transform)
{
    Vector3 localUp;
    localUp.x = 0.0f, localUp.y = 1.0f, localUp.z = 0.0f;

    Vector3 up = Vector3RotateByQuaternion(localUp, QuaternionNormalize(transform.rotation));

    up = Vector3Normalize(up);

    return up;
}

inline Vector3 GetWorldRightVector(const Transform& transform)
{
    Vector3 localRight;
    localRight.x = 1.0f, localRight.y = 0.0f, localRight.z = 0.0f;

    Vector3 right = Vector3RotateByQuaternion(localRight, QuaternionNormalize(transform.rotation));

    right = Vector3Normalize(right);

    return right;
}

inline void VecFollowTarget(Vector3* position, const Vector3& offset, const Transform& targetTransform)
{
    Vector3 rotatedOffset = Vector3RotateByQuaternion(offset,targetTransform.rotation);

    position->x = targetTransform.translation.x + rotatedOffset.x;
    position->y = targetTransform.translation.y + rotatedOffset.y;
    position->z = targetTransform.translation.z + rotatedOffset.z;
}

inline void FollowTransform(Transform* transform_1, const Transform& transform_2, const Vector3& offset)
{
    Vector3 rotatedOffset = Vector3RotateByQuaternion(offset,transform_2.rotation);

    transform_1->translation.x = transform_2.translation.x + rotatedOffset.x;
    transform_1->translation.y = transform_2.translation.y + rotatedOffset.y;
    transform_1->translation.z = transform_2.translation.z + rotatedOffset.z;

    transform_1->rotation = transform_2.rotation;
}

inline void RotateTowardsCamera(Transform* transform_1, const Camera& camera)
{
    Matrix m = MatrixLookAt(transform_1->translation, camera.position, {0,1,0});
    
    Matrix mWorld = MatrixInvert(m);

    transform_1->rotation = QuaternionFromMatrix(mWorld);
}

