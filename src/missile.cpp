#include "missile.h"
#include "plane.h"
#include <algorithm>
#include <iostream>

void Missile::UpdateMissile(float dt, float iterations)
{
    if(thrust < maxThrust) thrust += 500000 * (dt / iterations);
    else thrust = maxThrust;

    Vector3 forward = GetLocalForwardVector(body.transform);
    body.ApplyForce(forward, thrust);
    body.UpdateBody(dt, iterations);
}


std::vector<Missile> InitMissiles(int quantity, float lifetime, float lockDistance, float maxThrust)
{
    std::vector<Missile> missileArray;

    for (int i = 0; i < quantity; i++)
    {
        Missile tempMissile = Missile();
        
        tempMissile.lockDistance = lockDistance;
        
        tempMissile.locked = false;
        tempMissile.didHit = false;
        tempMissile.isAlive = false;

        tempMissile.thrust = 0.0f;
        tempMissile.lifetime = lifetime;
        tempMissile.currentTime = 0.0f;
        tempMissile.maxThrust = maxThrust;

        tempMissile.body = Body3D(4.5f, {3.0f,3.0f,3.0f});

        std::cout << "Init missile " << i << " isAlive=" << tempMissile.isAlive << " \n";
        missileArray.push_back(tempMissile);
    }

    return missileArray;
}

MissilePool::MissilePool(int quantity, float lifetime, float lockDistance, float maxThrust)
{
    this->missiles = InitMissiles(quantity, lifetime, lockDistance, maxThrust);

    for (Missile &missile : this->missiles)
    {
        if(missile.isAlive)
        {
            activeMissiles.push_back(&missile);
        }
        else
        {
            inactiveMissiles.push_back(&missile);
        }
    }
}

void MissilePool::UpdateMissiles(float dt, int iterations)
{
    float fdt = dt;
    fdt /= iterations;

    for (Missile* m : activeMissiles)
    {
        if(m->isAlive)
        {
            m->currentTime += fdt;

            if(m->currentTime >= m->lifetime)
            {
                m->isAlive = false;
            }

            if(m->didHit)
            {
                m->isAlive = false;
            }

            m->UpdateMissile(dt, iterations);
        }
    }

    auto it = std::remove_if(activeMissiles.begin(), activeMissiles.end(),
        [&](Missile* m)
        {
            if(!m->isAlive)
            {
                inactiveMissiles.push_back(m);
                return true;
            }
            return false;
        });
    activeMissiles.erase(it, activeMissiles.end());
    
}

void MissilePool::FireMissile(const Transform &transform, Vector3 initialSpeed, float initialThrust)
{
    if(!inactiveMissiles.empty())
    {
        Missile* m = inactiveMissiles.back();
        inactiveMissiles.pop_back();

        std::cout<<"inactive missiles "<< this <<" " <<inactiveMissiles.size()<<"\n";

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
