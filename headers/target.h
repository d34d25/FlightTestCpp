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
public:

    EnemyType type;

    Target target;

    BulletPool bulletPool;
    MissilePool missilePool;

    Enemy(Target target, EnemyType type);

    void UpdateEnemy(float dt, int iterations, const Vector3& playerPos, const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);

private:

    void UpdateAATank(float dt, int iterations, const Vector3& playerPos, const std::vector<Missile*>& activeMissilesA, const std::vector<Missile*>& activeMissilesB);
};