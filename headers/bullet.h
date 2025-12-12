#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <memory>

class Bullet
{
public:

    Transform transform;
    Vector3 linearVelocity;
    bool isAlive;

    float radius = 2.0f;

    bool didHit = false;

    float lifetime;
    float currentTime;

    Vector3 initialVel;
    Vector3 force;

    Bullet() = default;

    void UpdateBullet(float dt);
};

class BulletPool
{
public:

    std::vector<Bullet*> activeBullets;
    std::vector<Bullet*> inactiveBullets;
    std::vector<std::unique_ptr<Bullet>> bullets;

    BulletPool() = default;

    BulletPool(int quantity, float lifetime);

    void UpdateBullets(float dt);
    
    void FireBullet(const Transform& transform, const Vector3& initialVel);
};