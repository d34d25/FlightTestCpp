#pragma once

#include "simpleTransform.h"

extern const float FORWARD_DRAG_MULTIPLIER;

extern const float ALT_ANGULAR_DAMPING;

class Body3D
{
private:

    float density = 100.0f;

    float width = 1;
    float height = 1;
    float length = 1;
    
    float mass = INFINITY;
    Vector3 inertia;

    Vector3 linearAcceleration;
    Vector3 angularAcceleration;

    float linearDrag = 100.0f; //same as mass
    float angularDrag = 16.6667f; //same as inertia

public:

    Transform transform;

    Vector3 linearVelocity;
    Vector3 angularVelocity;

    float worldAngularTorque;
    float worldAngularSpeed;

    Vector3 alternateForce;
    Vector3 alternateLinearVelocity;

    Vector3 alternateTorque;
    Vector3 alternateAngularSpeed;

    Vector3 force;
    Vector3 torque;

    float lateralDragMultiplier;

    Vector3 angularDamping;

    bool isStatic = false;

    Body3D() = default;

    Body3D(float sideDrag, Vector3 angularDamping);

    inline void ApplyForce(Vector3 direction, float amount)
    {
        force.x += direction.x * amount;
        force.y += direction.y * amount;
        force.z += direction.z * amount;
    }

    inline void ApplyLocalForce(Vector3 direction, float amount)
    {
        Vector3 worldDirection = Vector3RotateByQuaternion(direction, transform.rotation);

        force.x += worldDirection.x * amount;
        force.y += worldDirection.y * amount;
        force.z += worldDirection.z * amount;
    }

    inline void ApplyPitch(float amount)
    {
        torque.x += amount;
    }

    inline void ApplyRoll(float amount)
    {
        torque.z += amount;
    }

    inline void ApplyYaw(float amount)
    {
        torque.y += amount;
    }

    inline void ApplyAlternateLocalForce(Vector3 direction, float amount)
    {
        Vector3 worldDirection = Vector3RotateByQuaternion(direction, transform.rotation);

        alternateForce.x += worldDirection.x * amount;
        alternateForce.y += worldDirection.y * amount;
        alternateForce.z += worldDirection.z * amount;
    }

    inline void ApplyAlternateYaw(float amount)
    {
        alternateTorque.y += amount;
    }

    inline void ApplyAlternatePitch(float amount)
    {
        alternateTorque.x += amount;
    }

    void AlternateUpdateBody(float dt, int iterations);

    void ApplyAlternateWorldTorque(Vector3 axis, float dt);

    void UpdateBody(float dt, int iterations);

    void SingleBodyUpdate(float dt, int iterations);

    //consults

    inline bool HasZeroMass()
    {
        return mass == 0.0f;
    }

    inline bool HasZeroInertia()
    {
        return inertia.x == 0.0f ||
        inertia.y == 0.0f || inertia.z == 0.0f;
    }

    inline bool HasInfiniteMass()
    {
        return mass == INFINITY;
    }

    inline bool HasInfiniteInertia()
    {
        return inertia.x == INFINITY ||
        inertia.y == INFINITY || inertia.z == INFINITY;
    }

    //getters
    inline float GetMass()
    {
        return mass;
    }

    inline Vector3 GetInertia()
    {
        return inertia;
    }

    inline float GetForwardDragFactor()
    {
        return linearDrag * FORWARD_DRAG_MULTIPLIER;
    }

    inline Vector3 GetTrueVelocity()
    {
        return Vector3Add(linearVelocity, alternateLinearVelocity);
    }
};