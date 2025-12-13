#pragma once

#include "plane.h"
#include "bullet.h"
#include "missile.h"
#include "target.h"
#include <memory>

class Player
{
private:

    bool stalling = false;

    float originalMaxPitchSpeed;
    float originalMaxRollSpeed;
    float originalMaxYawSpeed;

    float pitchInputUp;
    float rollInputRight;
    float yawInputRight;

    float pitchInputDown;
    float rollInputLeft;
    float yawInputLeft;

    bool globalCamera = false;
    float orbitYaw;
    float orbitPitch;
    float orbitDistance;

    Vector3 smoothedOffset;

    float engineGlow;
    float idleEngineGlow;

    int maxEngineGlow;
    float engineGlowChange;

    Transform bulletTransform;
    float fireTimerBullet;
    float firerateBullet;
    
    Transform missileTransformA;
    float fireTimerMissileA;
    float firerateMissile;

    float fireTimerMissileB;

    Transform missileTransformB;

    bool currentMissilePool;

    bool returnToIdle = true;
    bool hasInput = false;

    int tgtIndex;
    bool tgtLocked;
    
public:

    PlaneParams params;

    Body3D body;

    Camera3D camera;

    Vector3 cameraOffset;

    BulletPool bulletPool;

    MissilePool missilePoolA;

    MissilePool missilePoolB;

    float thrust = 0.0f;

    std::vector<std::weak_ptr<Target>> targets; 
    std::weak_ptr<Target> currentTarget;

    Player();

    void UpdatePlayer(float dt, int iterations);
    
    void UpdateCamera(float dt);

    void FireB(float dt);

    void FireM(float dt);

    void ChooseTarget();
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
    
    inline float GetTrueSpeed()
    {
        return Vector3Length(body.GetTrueVelocity());    
    }

    inline float GetScale()
    {
        return params.scale;
    }
};