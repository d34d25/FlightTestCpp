#include "target.h"
#include "raymath.h"
#include <iostream>
#include <plane.h>

//if this creates a segmentation fault use unique ptr for tgt
Target CreateTarget(Vector3 position, float width, float height, float length, float health, EnemyColliderType colliderType)
{
    Target tgt = {};

    tgt.body = Body3D();

    tgt.body.transform.translation = position;
    tgt.body.lateralDragMultiplier = 200;
    tgt.body.angularDamping = {3,3,3};

    tgt.hitbox = Collider();

    tgt.width = width;
    tgt.height = height;
    tgt.length = length;

    switch (colliderType)
    {
    case EnemyColliderType::BOX:
        tgt.hitbox.CreatePrismatoidUp(tgt.width, tgt.length, tgt.width, tgt.length, tgt.height);
        break;
    case EnemyColliderType::PRISMATOID_FORWARD:
        tgt.hitbox.CreatePrismatoidForward(tgt.width,tgt.height,tgt.width/2,tgt.height,tgt.length);
        break;
    case EnemyColliderType::PRISMATOID_UP:
        tgt.hitbox.CreatePrismatoidUp(tgt.width,tgt.length, tgt.width /2, tgt.length /2, tgt.height);
        break;
    default:
        tgt.hitbox.CreatePrismatoidUp(tgt.width, tgt.length, tgt.width, tgt.length, tgt.height);
        break;
    }
    tgt.health = health;

    tgt.isLocked = false;

    return tgt;
}

Enemy::Enemy(Target target, EnemyType type)
{
    bulletPool = BulletPool(30, 1, BULLET_DAMPING);
    missilePool = MissilePool(4, 2, MAX_THRUST * 0.75f);

    this->target = target;
    this->type = type;
    target.isLocked = false;
}

void Enemy::UpdateEnemy(float dt, int iterations, const Vector3 &playerPos, const std::vector<Missile *>& activeMissilesA, const std::vector<Missile *>& activeMissilesB)
{
    switch (type)
    {
    case EnemyType::AA_TANK:
        UpdateAATank(dt,iterations,playerPos, activeMissilesA, activeMissilesB);
        break;
    
    default:
        break;
    }
}

void Enemy::UpdateAATank(float dt, int iterations, const Vector3 &playerPos, const std::vector<Missile *>& activeMissilesA, const std::vector<Missile *>& activeMissilesB)
{
    target.isLocked = false;

    for (int i = 0; i < activeMissilesA.size(); i++)
    {
        Missile* currentMissile = activeMissilesA[i];

        if(!currentMissile) continue;
        
        if (currentMissile->target.x == target.body.transform.translation.x ||
            currentMissile->target.y == target.body.transform.translation.y ||
            currentMissile->target.z == target.body.transform.translation.z)
        {
            target.isLocked = true;
            break;
        }
    }

    for (int i = 0; i < activeMissilesB.size(); i++)
    {
        Missile* currentMissile = activeMissilesB[i];

        if(!currentMissile) continue;

        if (currentMissile->target.x == target.body.transform.translation.x ||
            currentMissile->target.y == target.body.transform.translation.y ||
            currentMissile->target.z == target.body.transform.translation.z)
        {
            target.isLocked = true;
            break;
        }
    }

    if (target.isLocked)
    {
        
    }

    Vector3 dirToPlayer = Vector3Subtract(playerPos, target.body.transform.translation);
    float distToPlayer = Vector3Length(dirToPlayer);

    Vector3 dirToPlayerNormalized = Vector3Normalize(dirToPlayer);

    if (distToPlayer <= 1500)
    {
       // Vector3 bulletVel = Vector3Add()
        
        //bulletPool.FireBullet(target.transform, )
    }

    target.body.UpdateBody(dt, iterations);
}
