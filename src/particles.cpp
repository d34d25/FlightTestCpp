#include "particles.h"
#include <algorithm>

void Particle::UpdateParticle(float dt)
{
    transform.translation.x += velocity.x * dt;
    transform.translation.y += velocity.y * dt;
    transform.translation.z += velocity.z * dt;

    radius += 5 * dt;

    alpha -= 100 * dt;
    if(alpha <= 0.0f) alpha = 0.0f;
}

ParticlePool::ParticlePool(int quantity, float lifetime)
{
    for (int i = 0; i < quantity; i++)
    {
        std::unique_ptr<Particle> tempParticle = std::make_unique<Particle>();

        tempParticle->isAlive = false;
        tempParticle->transform = {};
        tempParticle->lifetime = lifetime;
        tempParticle->velocity = {0,0,0};
        tempParticle->currentTime = 0.0f;
        tempParticle->alpha = 0.0f;

        tempParticle->radius = 10.0f;

        particles.push_back(std::move(tempParticle));
    }

    for (const auto& particlePtr : particles)
    {
        Particle* rawParticlePtr = particlePtr.get();
        if(rawParticlePtr->isAlive)
        {
            activeParticles.push_back(rawParticlePtr);
        }
        else
        {
            inactiveParticles.push_back(rawParticlePtr);
        }
    }
}

void ParticlePool::UpdateParticles(float dt)
{
    for (int i = 0; i < activeParticles.size();)
    {
        Particle* p = activeParticles[i];
        p->UpdateParticle(dt);

        p->currentTime += dt;

        if(p->currentTime >= p->lifetime || p->alpha <= 0.0f)
        {
            p->isAlive = false;
            inactiveParticles.push_back(p);

            activeParticles[i] = activeParticles.back();
            activeParticles.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void ParticlePool::FireParticle(const Transform &transform, const Vector3 &velocity)
{
    if (!inactiveParticles.empty())
    {
        Particle* p = inactiveParticles.back();
        inactiveParticles.pop_back();

        p->transform = transform;
        p->isAlive = true;
        p->currentTime = 0.0f;
        p->velocity = velocity;

        p->alpha = 110;
        p->radius = 5;

        activeParticles.push_back(p);
    }
}

void ParticlePool::ResetParticlePool()
{
    for(Particle* p : activeParticles)
    {
        p->isAlive = false;
        inactiveParticles.push_back(p);
    }
    activeParticles.clear();
}
