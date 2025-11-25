#pragma once

#include "simpleTransform.h"

class Body3D
{
private:

    float density = 100.0f;

    float width = 1;
    float height = 1;
    float length = 1;
    
    float mass = INFINITY;
    Vector3 inertia;

    float worldAngVel = 0;

    Vector3 linearAcceleration;
    Vector3 angularAcceleration;

public:

    Transform transform;

    Vector3 linearVelocity;
    Vector3 angularVelocity;

    Vector3 force;
    Vector3 torque;

    float linearDamping = 3;
    Vector3 angularDamping;

    bool isStatic = false;

    Body3D() = default;

    Body3D(float linearDamping, Vector3 angularDamping);

    void ApplyForce(Vector3 direction, float amount);

    void ApplyLocalForce(Vector3 direction, float amount);

    void ApplyPitch(float amount);

    void ApplyRoll(float amount);

    void ApplyYaw(float amount);

    void ApplyWorldTorque(float speed, float torque, Vector3 axis, float dt);

    void UpdateBody(float dt, int iterations);

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
};