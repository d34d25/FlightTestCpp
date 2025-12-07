#pragma once
#include "raylib.h"
#include "collider.h"

struct Target
{
    Transform transform;
    
    float width, height, lenght;
    Collider hitbox;

    float health;

    bool isLocked;
};

Target CreateTarget(Vector3 position, float width, float height, float length, float health);