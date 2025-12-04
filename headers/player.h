#pragma once

#include "plane.h"
#include "bullet.h"
#include "missile.h"

class Player
{
private:

    bool stalling = false;

    float originalMaxPitchSpeed;
    float originalMaxRollSpeed;
    float originalMaxYawSpeed;

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

    Transform bulletTransform;
    float fireTimerBullet = 0.0f;
    float firerateBullet = 0.1f;
    
    Transform missileTransformA;
    float fireTimerMissileA;
    float firerateMissile;

    float fireTimerMissileB;

    Transform missileTransformB;

    bool currentMissilePool;

    bool returnToIdle = true;
    bool hasInput = false;
    
public:

    PlaneParams params;

    Body3D body;

    Camera3D camera;

    Vector3 cameraOffset;

    BulletPool bulletPool;

    MissilePool missilePoolA;

    MissilePool missilePoolB;

    float thrust = 0.0f;

    Player();

    void UpdatePlayer(float dt, int iterations);
    
    void UpdateCamera(float dt);

    void FireB(float dt);

    void FireM(float dt);
    //getters

    inline float GetEngineGlow()
    {
        return engineGlow;
    }

    inline Body3D GetBody()
    {
        return body;
    }

    inline Transform GetTransform()
    {
        return body.transform;
    }

    inline Transform GetHitboxTransform()
    {
        Transform colliderTransform = {};

        FollowTransform(&colliderTransform, GetTransform(),{0,0,2});

        return colliderTransform;
    }

    inline Vector3 GetPosition()
    {
        return body.transform.translation;
    }

    inline Quaternion GetOrientation()
    {
        return body.transform.rotation;
    }

    inline int GetWidth()
    {
        return params.hitboxWidth;
    }

    inline int GetHeight()
    {
        return params.hitboxHeight;
    }

    inline int GetLength()
    {
        return params.hitboxLength;
    }
    
    inline float GetMaxSpeed()
    {
        float maxThrust = params.idleThrust + (params.maxThrust - params.idleThrust);

        float maxVelocity = maxThrust  / body.GetMass();

        float correctionFactor = 0.97f;

        return (maxVelocity / body.GetForwardDragFactor() * 100) * correctionFactor;
    }

    inline float GetIdleSpeed()
    {
        float maxVelocityIdle = params.idleThrust / body.GetMass();

        float correctionFactor = 0.97f;

        return(maxVelocityIdle / body.GetForwardDragFactor() * 100) * correctionFactor;
    }

    inline float GetSpeed()
    {
        return Vector3Length(body.linearVelocity);
    }

    inline float GetScale()
    {
        return params.scale;
    }
};