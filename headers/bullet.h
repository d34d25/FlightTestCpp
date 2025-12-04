#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <memory>

class Bullet
{
public:

    Transform transform;
    Vector3 velocityVec;
    float damping;
    bool isAlive;

    float radius = 2.0f;

    bool didHit = false;

    float lifetime;
    float currentTime;

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

    BulletPool(int quantity, float lifetime, float damping);

    void UpdateBullets(float dt);
    
    void FireBullet(const Transform& transform, const Vector3& force);
};