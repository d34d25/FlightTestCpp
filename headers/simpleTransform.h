#pragma once
#include "raylib.h"
#include "raymath.h"
#include <array>

Matrix GetLocalTransform(const Transform& transform);

std::array<float, 16> GetLocalMatrixTransform(const Transform& transform);

Vector3 GetLocalForwardVector(const Transform& transform);

Vector3 GetLocalUpVector(const Transform& transform);

Vector3 GetLocalRightVector(const Transform& transform);

//void VecFollowTarget(Vector3* position, const Vector3& offset, const Transform& targetTransform);

//void FollowTransform(Transform* transform_1, const Transform& transform_2, const Vector3& offset);