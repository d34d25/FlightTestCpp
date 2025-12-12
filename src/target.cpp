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
    bulletPool = BulletPool(30, 3);
    missilePool = MissilePool(4, 2, MAX_THRUST * 0.75f);

    fireTimerBullet = 0.0f;
    firerateBullet = 0.4f;

    fireTimerMissile = 0.0f;
    firerateMissile = 1.5f;

    this->target = target;
    this->type = type;
    target.isLocked = false;

    playerInRange = false;
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

void Enemy::FireB(float dt, const Vector3& playerPos, const Vector3& playerVel)
{
    if(playerInRange)
    {    
        int bulletspeed = 500;

        Vector3 enemyPos = target.body.transform.translation;

        Vector3 dirToPlayer = Vector3Subtract(playerPos, enemyPos);
        float distToPlayer = Vector3Length(dirToPlayer);

        float timeToImpact = distToPlayer / bulletspeed;

        Vector3 predictedOffset;
        Vector3 predictedPlayerPos;

        Vector3 dirToPredictedPos;

        for(int i = 0; i < 4; i++)
        {
            predictedOffset = Vector3Scale(playerVel, timeToImpact);
            predictedPlayerPos = Vector3Add(playerPos, predictedOffset);

            dirToPredictedPos = Vector3Subtract(predictedPlayerPos, enemyPos);
            
            float distToPredictedPos = Vector3Length(dirToPredictedPos);

            timeToImpact = distToPredictedPos / bulletspeed;            
        }

        Vector3 predictedDir = Vector3Normalize(dirToPredictedPos);

        Transform bulletTransform = {};

        bulletTransform.translation = enemyPos;
        bulletTransform.scale = target.body.transform.scale;
        
        Vector3 localForward = {0,0,1};
        Vector3 rotationAxis = Vector3CrossProduct(localForward, predictedDir);

        float dot = Vector3DotProduct(localForward, predictedDir);
        float angle = acosf(dot);

        bulletTransform.rotation = QuaternionFromAxisAngle(rotationAxis, angle);

        if(fireTimerBullet > 0.0f) fireTimerBullet -= dt;
        
        while(fireTimerBullet <= 0.0f)
        {
            Vector3 bulletVelocity = Vector3Add(target.body.linearVelocity, Vector3Scale(predictedDir, bulletspeed));

            bulletPool.FireBullet(bulletTransform, bulletVelocity);

            fireTimerBullet = firerateBullet;
        }
    }
}

void Enemy::FireM(float dt, const Vector3& playerPos)
{

}


void Enemy::UpdateAATank(float dt, int iterations, const Vector3 &playerPos, const std::vector<Missile *> &activeMissilesA, const std::vector<Missile *> &activeMissilesB)
{
    Vector3 dirToPlayer = Vector3Subtract(playerPos, target.body.transform.translation);

    target.isLocked = false;
    playerInRange = false;

    for (int i = 0; i < activeMissilesA.size(); i++)
    {
        Missile* currentMissile = activeMissilesA[i];

        if(!currentMissile) continue;
        
        if (currentMissile->target.x == target.body.transform.translation.x ||
            currentMissile->target.y == target.body.transform.translation.y ||
            currentMissile->target.z == target.body.transform.translation.z)
        {
            target.isLocked = true;
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
        }
    }

    if (target.isLocked)
    {
        
    }
    
    float distToPlayer = Vector3Length(dirToPlayer);

    if (distToPlayer <= 4000)
    {
        playerInRange = true;
    }

    target.body.UpdateBody(dt, iterations);

    bulletPool.UpdateBullets(dt / iterations);
}
