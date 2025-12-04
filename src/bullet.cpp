#include "bullet.h"
#include "simpleTransform.h"
#include <algorithm>
#include <iostream>

void Bullet::UpdateBullet(float dt)
{
    transform.translation.x += velocityVec.x * dt;
    transform.translation.y += velocityVec.y * dt;
    transform.translation.z += velocityVec.z * dt;
    
    velocityVec.y += -30.0 * dt;

    velocityVec.x *= 1 - damping * dt;
    velocityVec.y *= 1 - damping * dt;
    velocityVec.z *= 1 - damping * dt;
}

BulletPool::BulletPool(int quantity, float lifetime, float damping)
{
    for (int i = 0; i < quantity; i++)
    {
        std::unique_ptr<Bullet> tempBullet = std::make_unique<Bullet>();

        tempBullet->isAlive = false;
        tempBullet->transform = {};
        tempBullet->damping = damping;
        tempBullet->lifetime = lifetime;
        tempBullet->currentTime = 0.0f;
        tempBullet->force = {0,0,0};

        bullets.push_back(std::move(tempBullet));
    }

    for (const auto& bulletPtr : this->bullets)
    {
        Bullet* rawBulletPtr = bulletPtr.get();
        if(rawBulletPtr->isAlive)
        {
            activeBullets.push_back(rawBulletPtr);
        }
        else
        {
            inactiveBullets.push_back(rawBulletPtr);
        }
    }
}

void BulletPool::UpdateBullets(float dt)
{
    for (int i = 0; i < activeBullets.size();)
    {
        Bullet* b = activeBullets[i];
        b->UpdateBullet(dt);
        b->currentTime += dt;

        if(b->currentTime >= b->lifetime || 
            abs(Vector3Length(b->velocityVec)) <= 0.00001f||
            b->didHit)
        {
            b->isAlive = false;
            inactiveBullets.push_back(b);

            activeBullets[i] = activeBullets.back();
            activeBullets.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void BulletPool::FireBullet(const Transform &transform, const Vector3& force)
{
    if (!inactiveBullets.empty())
    {
        Bullet* b = inactiveBullets.back();
        inactiveBullets.pop_back();

        b->transform = transform;
        b->isAlive = true;
        b->didHit = false;
        b->currentTime = 0.0f;
        b->force = force;

        b->velocityVec = {force.x,force.y,force.z};

        activeBullets.push_back(b);
    }
}
