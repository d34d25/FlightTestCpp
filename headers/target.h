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
    AA_GUN,
    SAM
};

class Target
{
private:

    Vector3 predictedPos;
    Vector3 predictedDir;
    float bulletspeed;

    void UpdateAATank(float dt, const Vector3& playerPos,const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

    void UpdateSAM(float dt, const Vector3& playerPos,const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

    bool IsLockedByMissile(const std::vector<Missile*>& activeMissiles);

    Vector3 SolveIntercept(const Vector3& playerPos, const Vector3& playerVel, float dt);

    void FireBullet_AAGun(float dt, const Vector3 &playerPos, const Vector3& playerVel);

    void FireMissile_SAM(float dt, const Vector3& playerPos);

public:

    EnemyType type;

    Body3D body;
    
    float width, height, length;

    Collider hitbox;

    float health;

    bool isLocked;

    BulletPool bulletPool;
    float fireTimerBullet;
    float firerateBullet;

    MissilePool missilePool;
    float fireTimerMissile;
    float firerateMissile;

    float thrust;

    bool playerInRange;

    Target() = default;

    Target(Vector3 position, float width, float height, float length, float health, EnemyColliderType colliderType, EnemyType type);

    void UpdateEnemy(float dt, const Vector3& playerPos,const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

    void FireBullet(float dt, const Vector3 &playerPos, const Vector3& playerVel);

    void FireMissile(float dt, const Vector3& playerPos);

    inline Vector3 GetPosition()
    {
        return body.transform.translation;
    }

    inline Vector3 GetVelocity()
    {
        return body.GetTrueLinearVelocity();
    }
};