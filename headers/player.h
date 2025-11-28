#pragma once

#include "plane.h"
#include "bullet.h"

class Player
{
private:

    int pitchInput = 0;
    int rollInput = 0;
    int yawInput = 0;

    bool globalCamera = false;
    float orbitYaw = 0.0f;
    float orbitPitch = 0.0f;
    float orbitDistance = -100.0f;

    Vector3 smoothedOffset;

    float engineGlow = 0.5f;
    float idleEngineGlow = 0.5f;

    int maxEngineGlow = 1;
    float engineGlowChange;

    float fireTimer = 0.0f;
    float firerate = 0.1f;

public:

    Plane plane;

    Camera3D camera;

    Vector3 cameraOffset;

    BulletPool bulletPool;

    Player(float scale);

    void UpdatePlayer(float dt, int iterations);
    
    void UpdateCamera(float dt);

    void Fire(float dt);
    //getters

    inline float GetEngineGlow()
    {
        return engineGlow;
    }

    inline Body3D GetBody()
    {
        return plane.body;
    }

    inline Transform GetTransform()
    {
        return plane.body.transform;
    }

    inline Transform GetHitboxTransform()
    {
        Transform colliderTransform = {};

        FollowTransform(&colliderTransform, GetTransform(),{0,0,2});

        return colliderTransform;
    }

    inline Vector3 GetPosition()
    {
        return plane.body.transform.translation;
    }

    inline Quaternion GetOrientation()
    {
        return plane.body.transform.rotation;
    }

    inline int GetWidth()
    {
        return plane.params.hitboxWidth;
    }

    inline int GetHeight()
    {
        return plane.params.hitboxHeight;
    }

    inline int GetLength()
    {
        return plane.params.hitboxLength;
    }    
};