#pragma once

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "body.h"
#include "particles.h"
#include <memory>

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

    Vector3 target;
    bool lockedOnTarget;

    Missile() = default;

    void UpdateMissile(float dt, float iterations);

};

class MissilePool
{
public:

    std::vector<Missile*> activeMissiles;
    std::vector<Missile*> inactiveMissiles;
    std::vector<std::unique_ptr<Missile>> missiles;
    
    MissilePool() = default;

    MissilePool(int quantity, float lifetime, float maxTrhust);

    void UpdateMissiles(float dt, int iterations);

    void FireMissile(const Transform& transform, Vector3 initialSpeed, float initialThrust, Vector3 target, bool locked);
};

