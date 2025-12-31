#include "target.h"
#include "raymath.h"
#include <iostream>
#include <plane.h>


//private
void Target::UpdateAATank(float dt, const Vector3 &playerPos, const std::vector<Missile *> &activeMissilesA, const std::vector<Missile *> &activeMissilesB)
{
    body.ApplyForce({0,0,1}, thrust);

    bulletPool.UpdateBullets(dt);
}

void Target::UpdateSAM(float dt, const Vector3 &playerPos, const std::vector<Missile *> &activeMissilesA, const std::vector<Missile *> &activeMissilesB)
{
    missilePool.UpdateMissiles(dt);
}

bool Target::IsLockedByMissile(const std::vector<Missile *> &activeMissiles)
{
    return false;
}

Vector3 Target::SolveIntercept(const Vector3 &playerPos, const Vector3 &playerVel, float dt)
{
    Vector3 relativePos = playerPos - GetPosition();

    Vector3 relativeVel = playerVel - GetVelocity();

    float a = Vector3DotProduct(relativeVel, relativeVel) - (bulletspeed * bulletspeed);
    float b = 2 * Vector3DotProduct(relativeVel, relativePos);
    float c = Vector3DotProduct(relativePos, relativePos);

    float t = -1.0f;

    float det = (b*b) - (4 * a * c);

    if(abs(a) < 0.0001f) return playerPos;

    if (det <= 0.0f) return playerPos;

    float t1 = (-b + sqrtf(det)) / (2 * a);
    float t2 = (-b - sqrtf(det)) / (2 * a);

    if(t1 > 0.0f && t2 > 0.0f) t = fminf(t1,t2);
    else if (t1 > 0.0f) t = t1;
    else if (t2 > 0.0f) t = t2;

    if (t < 0.0f) return playerPos;

    return playerPos + (playerVel * t);
}

//public

Target::Target(Vector3 position, float width, float height, float length, float health, EnemyColliderType colliderType, EnemyType type)
{
    body = Body3D(4.0f, {3.0f,3.0f,3.0f});
    body.transform.translation = position;

    hitbox = Collider();

    this->width = width;
    this->height = height;
    this->length = length;

    thrust = 0.0f;

    this->type = type;

    switch (colliderType)
    {
    case EnemyColliderType::BOX:
        hitbox.CreatePrismatoidUp(width, length, width, length, height);
        break;
    case EnemyColliderType::PRISMATOID_FORWARD:
        hitbox.CreatePrismatoidForward(width,height,width/2,height,length);
        break;
    case EnemyColliderType::PRISMATOID_UP:
        hitbox.CreatePrismatoidUp(width,length, width /2, length /2, height);
        break;
    default:
        hitbox.CreatePrismatoidUp(width, length, width, length, height);
        break;
    }

    this->health = health;

    isLocked = false;

    bulletPool = BulletPool(60, 3);
    missilePool = MissilePool(4, 7, MAX_THRUST * 0.5f);

    fireTimerBullet = 0.0f;
    firerateBullet = 0.13f;

    fireTimerMissile = 0.0f;
    firerateMissile = 1.5f;

    playerInRange = false;

    predictedPos = {0,0,0};
    predictedDir = {0,0,0};

    bulletspeed = 1200;
}

void Target::UpdateEnemy(float dt, const Vector3 &playerPos, const std::vector<Missile *> &activeMissilesA, const std::vector<Missile *> &activeMissilesB)
{ 
    Vector3 dirToPlayer = Vector3Subtract(playerPos, body.transform.translation);

    isLocked = false;
    playerInRange = false;

    float distToPlayer = Vector3Length(dirToPlayer);

    if (distToPlayer <= 4000)
    {
        playerInRange = true;
    }

    switch (type)
    {
    case EnemyType::AA_GUN:
        UpdateAATank(dt,playerPos, activeMissilesA, activeMissilesB);
        break;
    case EnemyType::SAM:
        UpdateSAM(dt, playerPos, activeMissilesA, activeMissilesB);
    default:
        break;
    }

    body.UpdateBody(dt);
}

void Target::FireBullet(float dt, const Vector3 &playerPos, const Vector3 &playerVel)
{
    switch (type)
    {
    case EnemyType::AA_GUN:
        FireBullet_AAGun(dt, playerPos, playerVel);
        break;
    default:
        break;
    }
}

void Target::FireBullet_AAGun(float dt, const Vector3 &playerPos, const Vector3 &playerVel)
{
    if(playerInRange)
    {    
        predictedPos = SolveIntercept(playerPos, playerVel, dt);
        
        predictedDir = Vector3Subtract(predictedPos, GetPosition());
        predictedDir = Vector3Normalize(predictedDir);

        Transform bulletTransform = {};

        //bullet spawn point
        bulletTransform.translation = GetPosition();

        bulletTransform.scale = {1,1,1};
        
        //bullet rotation is just for visuals
        Vector3 localForward = {0,0,1};
        Vector3 rotationAxis = Vector3CrossProduct(localForward, predictedDir);

        float dot = Vector3DotProduct(localForward, predictedDir);
        float angle = acosf(dot);

        bulletTransform.rotation = QuaternionFromAxisAngle(rotationAxis, angle);

        //deviation
        Vector3 forward = GetWorldForwardVector(bulletTransform);
        Vector3 right = GetWorldRightVector(bulletTransform);
        Vector3 up = GetWorldUpVector(bulletTransform);

        float maxDeviation = 40.0f;

        float rightDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * maxDeviation;
        float upDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * maxDeviation;

        Vector3 deviation = Vector3Add(
            Vector3Scale(right, rightDeviation),
            Vector3Scale(up, upDeviation)
        );

        //firing the bullet
        if(fireTimerBullet > 0.0f) fireTimerBullet -= dt;
        
        while(fireTimerBullet <= 0.0f)
        {
            Vector3 bulletVelocity = Vector3Scale(predictedDir, bulletspeed);

            bulletVelocity = Vector3Add(bulletVelocity, deviation);

            bulletPool.FireBullet(bulletTransform, bulletVelocity);

            fireTimerBullet = firerateBullet;
        }
    }
}

void Target::FireMissile(float dt, const Vector3 &playerPos)
{
    switch (type)
    {
    case EnemyType::SAM:
        FireMissile_SAM(dt, playerPos);
        break;
    default:
        break;
    }
}

void Target::FireMissile_SAM(float dt, const Vector3 &playerPos)
{
    if(playerInRange)
    {
        Vector3 dir = playerPos - GetPosition();
        dir = Vector3Normalize(dir);

        Transform missileTransform = {};

        missileTransform.scale = {1,1,1};

        missileTransform.translation = GetPosition();

        //missile rotation (visuals)
        Vector3 localForward = {0,0,1};
        Vector3 rotationAxis = Vector3CrossProduct(localForward, dir);

        float dot = Vector3DotProduct(localForward, dir);
        float angle = acosf(dot);

        missileTransform.rotation =  QuaternionFromAxisAngle(rotationAxis, angle);

        //missile spawn
        if(fireTimerMissile > 0.0f) fireTimerMissile -= dt;

        if(fireTimerMissile <= 0.0f)
        {
            missilePool.FireMissile(missileTransform, GetVelocity(), thrust, &playerPos, true);

            fireTimerMissile = firerateMissile;
        }
        
    }
}
