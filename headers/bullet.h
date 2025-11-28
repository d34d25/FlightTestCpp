#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

class Bullet
{
public:

    Ray raycast;
    Transform transform;
    Vector3 velocityVec;
    float damping;
    bool isAlive;

    float lifetime;
    float currentTime;

    float force;

    Bullet() = default;

    void UpdateBullet(float dt);
};

std::vector<Bullet> InitBullets(int quantity, float lifetime ,float damping);

class BulletPool
{
public:

    std::vector<Bullet*> activeBullets;
    std::vector<Bullet*> inactiveBullets;
    std::vector<Bullet> bullets;

    BulletPool() = default;

    BulletPool(int quantity, float lifetime, float damping);

    void UpdateBullets(float dt);
    
    void FireBullet(const Transform& transform, float force);
};