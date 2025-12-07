#include "target.h"
#include "raymath.h"

//if this creates a segmentation fault use unique ptr for tgt
Target CreateTarget(Vector3 position, float width, float height, float length, float health)
{
    Target tgt = {};

    tgt.transform = {};

    tgt.transform.translation = position;
    tgt.transform.rotation = QuaternionIdentity();
    tgt.transform.scale = {1,1,1};

    tgt.hitbox = Collider();

    tgt.width = width;
    tgt.height = height;
    tgt.lenght = length;

    tgt.hitbox.CreatePrismatoidUp(tgt.width, tgt.lenght, tgt.width, tgt.lenght, tgt.height);

    tgt.health = health;

    tgt.isLocked = false;

    return tgt;
}