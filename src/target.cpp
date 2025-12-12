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
    firerateBullet = 0.2f;

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
        int bulletspeed = 1900;

        Vector3 predictedPos = SolveIntercept(playerPos, playerVel, bulletspeed);
        Vector3 predictedDir = Vector3Subtract(predictedPos, GetPosition());
        predictedDir = Vector3Normalize(predictedDir);

        Transform bulletTransform = {};

        bulletTransform.translation = GetPosition();
        bulletTransform.scale = {1,1,1};
        
        Vector3 localForward = {0,0,1};
        Vector3 rotationAxis = Vector3CrossProduct(localForward, predictedDir);

        float dot = Vector3DotProduct(localForward, predictedDir);
        float angle = acosf(dot);

        bulletTransform.rotation = QuaternionFromAxisAngle(rotationAxis, angle);

        Vector3 forward = GetWorldForwardVector(bulletTransform);
        Vector3 right = GetWorldRightVector(bulletTransform);
        Vector3 up = GetWorldUpVector(bulletTransform);

        float maxDeviation = 300.0f;

        float rightDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * maxDeviation;
        float upDeviation = (2.0f * ((float)rand() / RAND_MAX) - 1.0f) * maxDeviation;

        Vector3 deviation = Vector3Add(
                Vector3Scale(right, rightDeviation),
                Vector3Scale(up, upDeviation));

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

void Enemy::FireM(float dt, const Vector3& playerPos)
{

}


void Enemy::UpdateAATank(float dt, int iterations, const Vector3 &playerPos, const std::vector<Missile *> &activeMissilesA, const std::vector<Missile *> &activeMissilesB)
{
    Vector3 dirToPlayer = Vector3Subtract(playerPos, target.body.transform.translation);

    target.isLocked = false;
    playerInRange = false;

    //target.isLocked = IsLockedByMissile(activeMissilesA) || IsLockedByMissile(activeMissilesB);
    
    float distToPlayer = Vector3Length(dirToPlayer);

    if (distToPlayer <= 4000)
    {
        playerInRange = true;
    }

    target.body.UpdateBody(dt, iterations);

    bulletPool.UpdateBullets(dt / iterations);
}

bool Enemy::IsLockedByMissile(const std::vector<Missile *> &activeMissiles)
{
    for (int i = 0; i < activeMissiles.size(); i++)
    {
        Missile* currentMissile = activeMissiles[i];

        return (currentMissile->target.x == target.body.transform.translation.x ||
            currentMissile->target.y == target.body.transform.translation.y ||
            currentMissile->target.z == target.body.transform.translation.z);
    }

    return false;
}

Vector3 Enemy::SolveIntercept(const Vector3 &playerPos, const Vector3 &playerVel, int bulletspeed)
{
    float a = bulletspeed * bulletspeed - Vector3DotProduct(playerVel, playerVel);
    float b = 2 * Vector3DotProduct(playerVel, Vector3Subtract(playerPos, GetPosition()));
    float c = Vector3DotProduct(Vector3Subtract(playerPos, GetPosition()), Vector3Subtract(playerPos, GetPosition()));

    float time = 0.0f;

    if(bulletspeed > Vector3Length(playerVel))
    {
        time = (b + sqrtf(b*b+4*a*c)) / (2*a);
    }

    return Vector3Add(playerPos, Vector3Scale(playerVel, time));
}
