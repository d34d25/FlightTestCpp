#pragma once
#include "raylib.h"
#include "collider.h"
#include "bullet.h"
#include "missile.h"

enum class EnemyColliderType
{
    BOX,
    PRISMATOID_FORWARD,
    PRISMATOID_UP
};

enum class EnemyType
{
    AA_TANK
};

struct Target
{
    Body3D body;
    
    float width, height, length;

    Collider hitbox;

    float health;

    bool isLocked;
};

Target CreateTarget(Vector3 position, float width, float height, float length, float health, EnemyColliderType colliderType);

class Enemy
{
private:

    Vector3 predictedPos;
    Vector3 predictedDir;
    int bulletspeed;

public:

    EnemyType type;

    std::shared_ptr<Target> target;

    BulletPool bulletPool;
    float fireTimerBullet;
    float firerateBullet;

    MissilePool missilePool;
    float fireTimerMissile;
    float firerateMissile;

    bool playerInRange;

    Enemy(Target target, EnemyType type);

    void UpdateEnemy(float dt, const Vector3& playerPos,const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

    void FireB(float dt, const Vector3 &playerPos, const Vector3& playerVel);

    void FireM(float dt, const Vector3& playerPos);

    inline Vector3 GetPosition()
    {
        return target->body.transform.translation;
    }

    inline Vector3 GetVelocity()
    {
        return target->body.linearVelocity;
    }

private:

    void UpdateAATank(float dt, const Vector3& playerPos,const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

    bool IsLockedByMissile(const std::vector<Missile*>& activeMissiles);

    Vector3 SolveIntercept(const Vector3& playerPos, const Vector3& playerVel, int bulletspeed);
};