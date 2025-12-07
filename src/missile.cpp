#include "missile.h"
#include "plane.h"
#include <algorithm>
#include <iostream>


void Missile::UpdateMissile(float dt, float iterations)
{
    Vector3 forward = GetLocalForwardVector(body.transform);

    //missile guiding system

    if(lockedOnTarget)
    {   
        Vector3 distToTarget = Vector3Subtract(target, body.transform.translation);

        Vector3 dirToTarget = Vector3Normalize(distToTarget);

        float angleToTarget = Vector3DotProduct(forward,dirToTarget);

        if(angleToTarget >= 0.75f)
        {
            Vector3 axisOfRotation = Vector3CrossProduct(forward, dirToTarget);
            axisOfRotation = Vector3Normalize(axisOfRotation);

            body.worldAngularTorque = 10 * ALT_ANGULAR_DAMPING;

            body.ApplyAlternateWorldTorque(axisOfRotation, dt / iterations);
        }
    }

    //missile update
    if(thrust < maxThrust) thrust += 500000 * (dt / iterations);
    else thrust = maxThrust;

    body.ApplyForce(forward, thrust);
    body.UpdateBody(dt, iterations);

    //missile particles
    if(isAlive)
    {
        if(fireTimerParticle > 0.0f) fireTimerParticle -= (dt / iterations);

        if(fireTimerParticle <= 0.0f)
        {
            Vector3 forward = GetLocalForwardVector(body.transform);
            Vector3 particleVel = Vector3Add(body.linearVelocity, Vector3Scale(forward, 50));

            Vector3 right = GetLocalRightVector(body.transform);
            Vector3 up = GetLocalUpVector(body.transform);

            float rightDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * 4.0f;
            float upDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * 4.0f;

            Vector3 deviation = Vector3Add(
                Vector3Scale(right, rightDeviation),
                Vector3Scale(up, upDeviation));

            particleVel = Vector3Add(particleVel, deviation);    

            particlePool.FireParticle(body.transform, particleVel);

            fireTimerParticle = firerateParticle;
        }        
    }
   
    particlePool.UpdateParticles(dt / iterations);
}


MissilePool::MissilePool(int quantity, float lifetime, float maxThrust)
{
    for (int i = 0; i < quantity; i++)
    {
        // 1. Allocate a Missile object on the heap
        std::unique_ptr<Missile> tempMissile = std::make_unique<Missile>();
        
        // 2. Perform all initializations on the heap object
        tempMissile->locked = false;
        tempMissile->didHit = false;
        tempMissile->isAlive = false;

        tempMissile->thrust = 0.0f;
        tempMissile->lifetime = lifetime;
        tempMissile->currentTime = 0.0f;
        tempMissile->maxThrust = maxThrust;

        tempMissile->body = Body3D(4.5f, {3.0f,3.0f,3.0f});
        
        tempMissile->particlePool = ParticlePool(30,1); 
        tempMissile->fireTimerParticle = 0.0f;
        tempMissile->firerateParticle = 0.05f;

        tempMissile->target = {0,0,0};
        tempMissile->lockedOnTarget = false;

        // 3. Move the object into the main vector (no copy occurs)
        this->missiles.push_back(std::move(tempMissile));
    }

    // 4. Populate active/inactive lists using raw pointers from the smart pointers
    for (const auto& missilePtr : this->missiles)
    {
        Missile* rawMissilePtr = missilePtr.get();
        if(rawMissilePtr->isAlive)
        {
            activeMissiles.push_back(rawMissilePtr);
        }
        else
        {
            inactiveMissiles.push_back(rawMissilePtr);
        }
    }
}

void MissilePool::UpdateMissiles(float dt, int iterations)
{
    float fdt = dt;
    fdt /= iterations;

    for (int i = 0; i < activeMissiles.size();)
    {
        Missile* m = activeMissiles[i];
        m->UpdateMissile(dt, iterations);

        m->currentTime += fdt;

        if(m->currentTime >= m->lifetime || m->didHit)
        {
            m->isAlive = false;
            m->lockedOnTarget = false;
            inactiveMissiles.push_back(m);

            activeMissiles[i] = activeMissiles.back();
            activeMissiles.pop_back();
        }
        else
        {
            i++;
        }
    }    
}

void MissilePool::FireMissile(const Transform &transform, Vector3 initialSpeed, float initialThrust, Vector3 target, bool locked)
{
    if(!inactiveMissiles.empty())
    {
        Missile* m = inactiveMissiles.back();
        inactiveMissiles.pop_back();

        m->particlePool.ResetParticlePool();

        m->lockedOnTarget = locked;

        m->target = target;  

        m->body.transform = transform;
        m->isAlive = true;
        m->didHit = false;
        m->currentTime = 0.0f;

        m->body.linearVelocity = {0,0,0};
        m->body.angularVelocity = {0,0,0};

        m->thrust = initialThrust;
        m->body.linearVelocity = {initialSpeed.x, initialSpeed.y, initialSpeed.z};

        Vector3 worldUp = GetLocalUpVector(m->body.transform);
        m->body.linearVelocity = Vector3Add(m->body.linearVelocity, Vector3Scale(worldUp, -7.0f));
        
        activeMissiles.push_back(m);
    }
}
