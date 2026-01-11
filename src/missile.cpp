#include "missile.h"
#include "plane.h"
#include <algorithm>
#include <iostream>

extern const float MISSILE_MOBILITY = 10.0f;

extern const float MISSILE_LOCK_ON_ANGLE = 0.6f;

void Missile::UpdateMissile(float dt)
{
    Vector3 forward = GetWorldForwardVector(body.transform);

    //missile guiding system

    if(lockedOnTarget)
    {   
        if(target)
        {
            Vector3 distToTarget = Vector3Subtract(*target, body.transform.translation);

            Vector3 dirToTarget = Vector3Normalize(distToTarget);

            float angleToTarget = Vector3DotProduct(forward,dirToTarget);

            if(angleToTarget >= lockOnAngle)
            {
                Vector3 axisOfRotation = Vector3CrossProduct(forward, dirToTarget);
                axisOfRotation = Vector3Normalize(axisOfRotation);

                body.worldAngularTorque = mobility * ALT_ANGULAR_DAMPING;

                body.ApplyAlternateWorldTorque(axisOfRotation, dt);
            }
        }
    }

    //missile update
    if(thrust < maxThrust) thrust += 500000 * dt;
    else thrust = maxThrust;

    body.ApplyForce(forward, thrust);
    body.UpdateBody(dt);

    //missile particles
    if(isAlive)
    {
        if(fireTimerParticle > 0.0f) fireTimerParticle -= dt;

        if(fireTimerParticle <= 0.0f)
        {
            Vector3 forward = GetWorldForwardVector(body.transform);
            Vector3 particleVel = Vector3Add(body.GetAbsoluteVelocity(), Vector3Scale(forward, 4000));

            particleVel *= dt;

            Vector3 right = GetWorldRightVector(body.transform);
            Vector3 up = GetWorldUpVector(body.transform);

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
   
    particlePool.UpdateParticles(dt);
}


MissilePool::MissilePool(int quantity, float lifetime, float maxThrust, float lockOnAngle, float mobility)
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
        
        tempMissile->particlePool = ParticlePool(80,0.25f); 
        tempMissile->fireTimerParticle = 0.0f;
        tempMissile->firerateParticle = 0.05f;

        tempMissile->target = nullptr;
        tempMissile->lockedOnTarget = false;

        tempMissile->lockOnAngle = lockOnAngle;
        tempMissile->mobility = mobility;

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

void MissilePool::UpdateMissiles(float dt)
{

    for (int i = 0; i < activeMissiles.size();)
    {
        Missile* m = activeMissiles[i];
        m->UpdateMissile(dt);

        m->currentTime += dt;

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

void MissilePool::FireMissile(const Transform &transform, Vector3 initialSpeed, float initialThrust, const Vector3* target, bool locked)
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

        Vector3 worldUp = GetWorldUpVector(m->body.transform);
        m->body.linearVelocity = Vector3Add(m->body.linearVelocity, Vector3Scale(worldUp, -7.0f));
        
        activeMissiles.push_back(m);
    }
}
