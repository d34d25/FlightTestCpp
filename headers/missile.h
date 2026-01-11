#pragma once

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "body.h"
#include "particles.h"
#include <memory>

extern const float MISSILE_MOBILITY;

extern const float MISSILE_LOCK_ON_ANGLE;


class Missile
{
public:

    bool locked;
    bool didHit;
    bool isAlive;

    float thrust;
    float maxThrust;

    float lifetime;
    float currentTime;

    float radius = 3.0f;

    Body3D body;
    
    ParticlePool particlePool;
    float fireTimerParticle;
    float firerateParticle;

    const Vector3* target;
    bool lockedOnTarget;

    float lockOnAngle;
    float mobility;

    Missile() = default;

    void UpdateMissile(float dt);

};

class MissilePool
{
public:

    std::vector<Missile*> activeMissiles;
    std::vector<Missile*> inactiveMissiles;
    std::vector<std::unique_ptr<Missile>> missiles;
    
    MissilePool() = default;

    MissilePool(int quantity, float lifetime, float maxThrust, float lockOnAngle, float mobility);

    void UpdateMissiles(float dt);

    void FireMissile(const Transform& transform, Vector3 initialSpeed, float initialThrust, const Vector3* target, bool locked);
};

