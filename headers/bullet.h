#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <memory>

extern const float BULLET_DAMPING;
extern const float BULLET_GRAVITY;

class Bullet
{
public:

    Transform transform;
    Vector3 linearVelocity;
    float damping;
    bool isAlive;

    float radius = 2.0f;

    bool didHit = false;

    float lifetime;
    float currentTime;

    Vector3 initialVel;
    Vector3 force;

    float gravitiy;

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

    BulletPool(int quantity, float lifetime, float damping, float gravity);

    void UpdateBullets(float dt);
    
    void FireBullet(const Transform& transform, const Vector3& initialVel);
};