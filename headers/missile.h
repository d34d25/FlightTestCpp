#pragma once

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "body.h"

class Missile
{
public:

    float lockDistance;

    bool locked;
    bool didHit;
    bool isAlive;

    float thrust;
    float lifetime;
    float currentTime;

    float hitDistance; //minimum distance that the missile has to be from the enmey to count a hit

    Body3D body;

    Missile() = default;

    void UpdateMissile(float dt, float iterations);

};


std::vector<Missile> InitMissiles(int quantity, float lifetime, float lockDistance, float hitDistance ,float thrust);

class MissilePool
{
public:

    std::vector<Missile*> activeMissiles;
    std::vector<Missile*> inactiveMissiles;
    std::vector<Missile> missiles;
    
    MissilePool() = default;

    MissilePool(int quantity, float lifetime, float lockDistance, float hitDistance ,float thrust);

    void UpdateMissiles(float dt, int iterations);

    void FireMissile(const Transform& transform, float initialSpeed);
};

