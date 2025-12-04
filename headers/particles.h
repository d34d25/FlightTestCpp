#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <memory>

class Particle
{
public:
    Transform transform;
    Vector3 velocity;

    bool isAlive;
    float lifetime;
    float currentTime;

    float alpha;
    float radius;

    Particle() = default;

    void UpdateParticle(float dt);
};

class ParticlePool
{
public:

    std::vector<Particle*> activeParticles;
    std::vector<Particle*> inactiveParticles;
    std::vector<std::unique_ptr<Particle>> particles;

    ParticlePool() = default;

    ParticlePool(int quantity, float lifetime);

    void UpdateParticles(float dt);

    void FireParticle(const Transform& transform, const Vector3& velocity);

    void ResetParticlePool();
};