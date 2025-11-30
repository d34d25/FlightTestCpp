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

std::vector<Bullet> InitBullets(int quantity, float lifetime ,float damping)
{
    std::vector<Bullet> bulletArray;

    for (int i = 0; i < quantity; i++)
    {
        Bullet tempBullet = Bullet();

        tempBullet.isAlive = false;
        tempBullet.transform = {};
        tempBullet.damping = damping;
        tempBullet.lifetime = lifetime;
        tempBullet.currentTime = 0.0f;
        tempBullet.force = {0,0,0};

        bulletArray.push_back(tempBullet);
    }

    return bulletArray;
}

BulletPool::BulletPool(int quantity, float lifetime, float damping)
{
    this->bullets = InitBullets(quantity, lifetime, damping);

    for (Bullet &bullet : this->bullets)
    {
        if(bullet.isAlive)
        {
            activeBullets.push_back(&bullet);
        }
        else
        {
            inactiveBullets.push_back(&bullet);
        }
    }
}

void BulletPool::UpdateBullets(float dt)
{
    for (Bullet* b : activeBullets)
    {
        if (b->isAlive)
        {
            b->currentTime += dt;
            if (b->currentTime >= b->lifetime)
            {
                b->isAlive = false;
            }

            if(abs(Vector3Length(b->velocityVec)) <= 0.0f)
            {
                b->isAlive = false;
            }

            if(b->didHit)
            {
                b->isAlive = false;
            }

            b->UpdateBullet(dt);
        }
    }

    // Move dead bullets back to inactive
    auto it = std::remove_if(activeBullets.begin(), activeBullets.end(),
        [&](Bullet* b) {
            if (!b->isAlive)
            {
                inactiveBullets.push_back(b);
                return true; // remove from active
            }
            return false;
        });

    activeBullets.erase(it, activeBullets.end());
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

        b->velocityVec = { force.x,force.y,force.z};

        activeBullets.push_back(b);
    }
}
