#include "bullet.h"
#include "simpleTransform.h"
#include <algorithm>
#include <iostream>

void Bullet::UpdateBullet(float dt)
{
    linearVelocity.x += force.x * dt;
    linearVelocity.y += force.y * dt;
    linearVelocity.z += force.z * dt;

    transform.translation.x += linearVelocity.x * dt;
    transform.translation.y += linearVelocity.y * dt;
    transform.translation.z += linearVelocity.z * dt;

    force.x = 0;
    force.y = 0;
    force.z = 0;
}

BulletPool::BulletPool(int quantity, float lifetime)
{
    for (int i = 0; i < quantity; i++)
    {
        std::unique_ptr<Bullet> tempBullet = std::make_unique<Bullet>();

        tempBullet->isAlive = false;
        tempBullet->transform = {};
        tempBullet->lifetime = lifetime;
        tempBullet->currentTime = 0.0f;
        tempBullet->initialVel = {0,0,0};
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
            abs(Vector3Length(b->linearVelocity)) <= 0.00001f||
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

void BulletPool::FireBullet(const Transform &transform, const Vector3& initialVel)
{
    if (!inactiveBullets.empty())
    {
        Bullet* b = inactiveBullets.back();
        inactiveBullets.pop_back();

        b->transform = transform;
        b->isAlive = true;
        b->didHit = false;
        b->currentTime = 0.0f;
        b->initialVel = initialVel;

        b->force = {0,0,0};
        b->linearVelocity = {initialVel.x,initialVel.y,initialVel.z};

        activeBullets.push_back(b);
    }
}
