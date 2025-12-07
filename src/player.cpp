#include "player.h"
#include <cmath>
#include <iostream>

const Vector3 forwardDir = {0.0f, 0.0f, 1.0f};

const Vector3 upDir = {0.0f, 1.0f, 0.0f};

const Vector3 backDir = {0.0f, 0.0f, -1.0f};

const Vector3 rightDir = {1.0f, 0.0f, 0.0f};

const Vector3 downDir = {0.0f, -1.0f, 0.0f};

const float FAKE_GRAVITY_FORCE = 7500 * FORWARD_DRAG_MULTIPLIER;
const float STALL_DOWNWARD_FORCE = 4000 * FORWARD_DRAG_MULTIPLIER;

const float MAX_BANK_TORQUE_YAW = 1.5f * ALT_ANGULAR_DAMPING;
const float MAX_BANK_TORQUE_PITCH = 1.5f * ALT_ANGULAR_DAMPING;

const float STALL_TORQUE_SPEED = 15.0f * ALT_ANGULAR_DAMPING;

Player::Player()
{
    params = GetPlaneParams(Planes::SF15);

    originalMaxPitchSpeed = params.maxPitchSpeed;
    originalMaxRollSpeed = params.maxRollSpeed;
    originalMaxYawSpeed = params.maxYawSpeed;

    body = Body3D(this->params.lateralDragMultiplier,
                  this->params.angularDamping);

    body.transform.translation.x = this->params.position.x;
    body.transform.translation.y = this->params.position.y;
    body.transform.translation.z = this->params.position.z;

    body.transform.scale = {params.scale, params.scale, params.scale};

    thrust = params.idleThrust;
    body.linearVelocity.z = 360;

    // camera
    camera.fovy = 60.0f;
    camera.up.x = 0.0f;
    camera.up.y = -1.0f;
    camera.up.z = 0.0f;

    camera.target = (Vector3){0, 0, 0};
    camera.position = (Vector3){0, 0, 0};

    camera.projection = CAMERA_PERSPECTIVE;

    cameraOffset.x = 0.0f;
    cameraOffset.y = 5.0f;
    cameraOffset.z = -39.0f;

    pitchInput = 0;
    rollInput = 0;
    yawInput = 0;

    globalCamera = false;
    orbitYaw = 0.0f;
    orbitPitch = 0.0f;
    orbitDistance = -100.0f;

    smoothedOffset.x = 0.0f;
    smoothedOffset.y = 0.0f;
    smoothedOffset.z = 0.0f;

    // engine glow
    engineGlow = 0.5f;
    idleEngineGlow = 0.5f;

    maxEngineGlow = 1;

    engineGlowChange = 0.2f;

    bulletPool = BulletPool(60, 1, 0.05f);
    bulletTransform = {};
    bulletTransform.scale = {1.0f, 1.0f, 1.0f};

    fireTimerMissileA = 0.0f;
    firerateMissile = 1.0f;

    fireTimerMissileB = 0.0f;

    missilePoolA = MissilePool(6, 4, MAX_THRUST * 1.25f);
    missileTransformA = {};
    missileTransformA.scale = {1.0f, 1.0f, 1.0f};
    missileTransformA.rotation = QuaternionIdentity();

    missilePoolB = MissilePool(6, 4, MAX_THRUST * 1.25f);
    missileTransformB = {};
    missileTransformB.scale = {1.0f, 1.0f, 1.0f};
    missileTransformB.rotation = QuaternionIdentity();

    currentMissilePool = false;


    //targets

    targets = {};
    currentTarget = nullptr;

    tgtIndex = 0;
    tgtLocked = false;
}

void Player::UpdatePlayer(float dt, int iterations)
{
    float fdt = dt;
    fdt /= iterations;

    Vector3 forward = GetLocalForwardVector(body.transform);
    forward = Vector3Normalize(forward);
    float forwardSpeed = Vector3DotProduct(body.linearVelocity, forward);

    if (!globalCamera)
    {
        if (IsKeyDown(KEY_W))
        {
            thrust += params.acceleration * fdt;

            engineGlow += engineGlowChange * fdt;

            hasInput = true;
        }
        else if (IsKeyDown(KEY_S))
        {
            thrust -= params.brake * fdt;

            engineGlow -= engineGlowChange * fdt;

            hasInput = true;
        }
        else
        {
            hasInput = false;

            if (engineGlow < idleEngineGlow)
            {
                engineGlow += engineGlowChange * fdt;
            }
            else if (engineGlow > idleEngineGlow)
            {
                engineGlow -= engineGlowChange * fdt;
            }
            else
            {
                engineGlow = idleEngineGlow;
            }
        }

        engineGlow = Clamp(engineGlow, 0, maxEngineGlow);

        if (IsKeyDown(KEY_D)) yawInput = -1;
        else if (IsKeyDown(KEY_A)) yawInput = 1;
        else yawInput = 0;

        if (IsKeyDown(KEY_RIGHT)) rollInput = 1;
        else if (IsKeyDown(KEY_LEFT)) rollInput = -1;
        else rollInput = 0;

        if (IsKeyDown(KEY_UP)) pitchInput = 1;
        else if (IsKeyDown(KEY_DOWN)) pitchInput = -1;
        else pitchInput = 0;
    }
    else
    {
        engineGlow = thrust / params.maxThrust;

        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            thrust += params.acceleration * fdt;
        }
        else if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            thrust -= params.brake * fdt;
        }

        if (IsKeyDown(KEY_E)) yawInput = -1;
        else if (IsKeyDown(KEY_Q)) yawInput = 1;
        else yawInput = 0;

        if (IsKeyDown(KEY_D)) rollInput = 1;
        else if (IsKeyDown(KEY_A)) rollInput = -1;
        else rollInput = 0;

        if (IsKeyDown(KEY_W)) pitchInput = 1;
        else if (IsKeyDown(KEY_S)) pitchInput = -1;
        else pitchInput = 0;
    }

    body.ApplyPitch(params.pitchPower * pitchInput * body.angularDamping.x);
    body.ApplyRoll(params.rollPower * rollInput * body.angularDamping.z);
    body.ApplyYaw(params.yawPower * yawInput * body.angularDamping.y);

    // plane

    if (body.angularVelocity.x >= params.maxPitchSpeed) body.angularVelocity.x = params.maxPitchSpeed;
    else if (body.angularVelocity.x <= -params.maxPitchSpeed) body.angularVelocity.x = -params.maxPitchSpeed;

    if (body.angularVelocity.y >= params.maxYawSpeed) body.angularVelocity.y = params.maxYawSpeed;
    else if (body.angularVelocity.y <= -params.maxYawSpeed) body.angularVelocity.y = -params.maxYawSpeed;

    if (body.angularVelocity.z >= params.maxRollSpeed) body.angularVelocity.z = params.maxRollSpeed;
    else if (body.angularVelocity.z <= -params.maxRollSpeed) body.angularVelocity.z = -params.maxRollSpeed;

    if (!hasInput && returnToIdle)
    {
        if (thrust <= params.idleThrust)
        {
            thrust += params.returnSpeedLow * fdt;
            if (thrust >= params.idleThrust) thrust = params.idleThrust;
        }
        else
        {
            thrust -= params.returnSpeedHigh * fdt;
            if (thrust <= params.idleThrust) thrust = params.idleThrust;
        }
    }
    else
    {
        if (thrust >= params.maxThrust) thrust = params.maxThrust;
        else if (thrust <= 0.0f) thrust = 0.0f;
    }

    body.ApplyLocalForce(forwardDir, thrust);

    // drag / fake gravity

    float dotFU = Vector3DotProduct(forward, upDir);

    if (dotFU > 0.1)
    {
        if (forwardSpeed > 0.0f) body.ApplyLocalForce(backDir, FAKE_GRAVITY_FORCE * dotFU);
    }
    else if (dotFU < -0.1)
    {
        body.ApplyLocalForce(backDir, FAKE_GRAVITY_FORCE * dotFU);
    }

    // fake banking

    Vector3 right = GetLocalRightVector(body.transform);
    right = Vector3Normalize(right);

    float rDot = Vector3DotProduct(upDir, right);

    body.ApplyAlternateYaw(MAX_BANK_TORQUE_YAW * -rDot);

    // upside down case

    Vector3 up = GetLocalUpVector(body.transform);
    up = Vector3Normalize(up);

    float uDot = Vector3DotProduct(upDir, up);

    if (uDot <= -0.1)
    {
        body.ApplyAlternatePitch(MAX_BANK_TORQUE_PITCH * uDot);
    }

    // fake stall

    float speed = GetSpeed();

    float fDot = Vector3DotProduct(downDir, forward);

    float fDotTarget = 0.6f;

    if (speed < params.stallSpeed)
    {
        stalling = true;
    }
    else if (speed >= params.recoverySpeed)
    {
        stalling = false;
    }

    float factor = 1.0f;

    if (speed <= params.mobilityLooseStartSpeed)
    {
        factor = 1 - (1 - params.proportionLow) * (speed - params.mobilityLooseStartSpeed) / (params.stallSpeed - params.mobilityLooseStartSpeed);
        factor = Clamp(factor, params.proportionLow, 1.0f);
    }
    else
    {
        factor = 1 - (1 - params.proportionHigh) * (speed - params.mobilityLooseStartSpeed) / (GetMaxSpeed() - params.mobilityLooseStartSpeed);
        factor = Clamp(factor, params.proportionHigh, 1.0f);
    }

    params.maxPitchSpeed = originalMaxPitchSpeed * factor;
    params.maxRollSpeed = originalMaxRollSpeed * factor;
    params.maxYawSpeed = originalMaxYawSpeed * factor;

    if (stalling)
    {
        body.torque.x = 0.0f;
        body.torque.y = 0.0f;
        body.torque.z = 0.0f;

        body.alternateTorque.x = 0.0f;
        body.alternateTorque.y = 0.0f;
        body.alternateTorque.z = 0.0f;

        body.alternateForce.y = -STALL_DOWNWARD_FORCE;
    }
    else
    {
        body.alternateForce.y = 0.0f;
    }

    Vector3 axisOfRotation = Vector3CrossProduct(forward, downDir);

    axisOfRotation = Vector3Normalize(axisOfRotation);

    if (stalling && fDot < fDotTarget)
    {
        body.worldAngularTorque = STALL_TORQUE_SPEED;
    }

    body.ApplyAlternateForce(fdt);
    body.ApplyAlernateTorque(fdt);
    body.ApplyAlternateWorldTorque(axisOfRotation, fdt);

    body.UpdateBody(dt, iterations);

    bulletPool.UpdateBullets(fdt);

    missilePoolA.UpdateMissiles(dt, iterations);
    missilePoolB.UpdateMissiles(dt,iterations);
}

void Player::UpdateCamera(float dt)
{
    HideCursor(); // when I have scenes this should be called once in the init of the scene

    Vector3 rotatedOffset = Vector3RotateByQuaternion(cameraOffset, GetOrientation());

    if (IsKeyPressed(KEY_ONE))
        globalCamera = !globalCamera;

    if (!globalCamera)
    {
        returnToIdle = true;

        float alpha = 10.0f * dt;

        smoothedOffset.x += alpha * (rotatedOffset.x - smoothedOffset.x);
        smoothedOffset.y += alpha * (rotatedOffset.y - smoothedOffset.y);
        smoothedOffset.z += alpha * (rotatedOffset.z - smoothedOffset.z);

        camera.position.x = GetPosition().x + smoothedOffset.x;
        camera.position.y = GetPosition().y + smoothedOffset.y;
        camera.position.z = GetPosition().z + smoothedOffset.z;

        Vector3 localForward = {0.0f, 0.14f, 1.0f};

        Vector3 forward = Vector3RotateByQuaternion(localForward, GetOrientation());

        int lookAhead = 200;

        camera.target.x = GetPosition().x + forward.x * lookAhead;
        camera.target.y = GetPosition().y + forward.y * lookAhead;
        camera.target.z = GetPosition().z + forward.z * lookAhead;

        Vector3 localUp = {0.0f, 1.0f, 0.0f};

        Vector3 upWorld = upWorld = Vector3RotateByQuaternion(localUp, GetOrientation());

        camera.up.x = upWorld.x;
        camera.up.y = upWorld.y;
        camera.up.z = upWorld.z;
    }
    else
    {
        returnToIdle = false;

        hasInput = true;

        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;

        Vector2 delta = GetMouseDelta();

        float wheel = GetMouseWheelMove();

        orbitDistance += wheel * 2.0f;
        orbitDistance = Clamp(orbitDistance, -200.0f, -10.0f);

        orbitYaw -= delta.x * 0.4f * dt;
        orbitPitch -= delta.y * 0.4f * dt;

        orbitPitch = Clamp(orbitPitch, -1.4f, 1.4f);

        float cosPitch = cos(orbitPitch);
        float sinPitch = sin(orbitPitch);
        float cosYaw = cos(orbitYaw);
        float sinYaw = sin(orbitYaw);

        Vector3 target = GetPosition();

        camera.position.x = target.x + orbitDistance * cosPitch * sinYaw;
        camera.position.y = target.y + orbitDistance * sinPitch;
        camera.position.z = target.z + orbitDistance * cosPitch * cosYaw;

        camera.target.x = target.x;
        camera.target.y = target.y;
        camera.target.z = target.z;

        camera.up = {0.0f, 1.0f, 0.0f};

        SetMousePosition(centerX, centerY);
    }
}

//call once per frame
void Player::FireB(float dt)
{
    int bulletspeed = 800; // 800

    // make this an attribute of the plane
    // call it gun position
    if (fireTimerBullet > 0.0f) fireTimerBullet -= dt;

    bool fireKey = (!globalCamera && IsKeyDown(KEY_LEFT_SHIFT)) || (globalCamera && IsKeyDown(KEY_SPACE));

    if (fireKey && fireTimerBullet <= 0.0f)
    {
        while (fireTimerBullet <= 0.0f)
        {
            FollowTransform(&bulletTransform, GetTransform(), {-2.75f, 0.9f, 10.0f});

            Vector3 forward = GetLocalForwardVector(body.transform);
            Vector3 bulletVelocity = Vector3Add(body.linearVelocity, Vector3Scale(forward, bulletspeed));

            bulletPool.FireBullet(bulletTransform, bulletVelocity);

            fireTimerBullet = firerateBullet;
        }
    }
}

//call once per frame
void Player::FireM(float dt)
{
    bool locked = false;
    Vector3 tgtPos = {0,0,0};

    if(currentTarget && tgtLocked)
    {
        locked = true;
        tgtPos = currentTarget->transform.translation;
    }

    if (fireTimerMissileA > 0.0f) fireTimerMissileA -= dt;
    if (fireTimerMissileB > 0.0f) fireTimerMissileB -= dt;

    bool fireKey = (!globalCamera && IsKeyPressed(KEY_SPACE)) || (globalCamera && IsKeyPressed(KEY_LEFT_ALT));

    if(fireKey)
    {
        Vector3 forward = GetLocalForwardVector(body.transform);
        Vector3 missileInitialSpeed = Vector3Scale(forward, GetSpeed());

        if (!currentMissilePool && fireTimerMissileA <= 0.0f)
        {
            FollowTransform(&missileTransformA, GetTransform(), {-4, -1, 0});
            missilePoolA.FireMissile(missileTransformA, missileInitialSpeed, thrust, tgtPos, locked);    
            
            fireTimerMissileA = firerateMissile;

            currentMissilePool = true;
        }
        else if (currentMissilePool && fireTimerMissileB <= 0.0f)
        {
            FollowTransform(&missileTransformB, GetTransform(), {4, -1, 0});
            missilePoolB.FireMissile(missileTransformB, missileInitialSpeed, thrust, tgtPos, locked);

            fireTimerMissileB = firerateMissile;

            currentMissilePool = false;
        }
    }    
}

void Player::ChooseTarget()
{
    float distToTgtMag = INFINITY;

    for (int i = 0; i < targets.size(); i++)
    {
        Vector3 distToTgt = Vector3Subtract(
            targets[i]->transform.translation, GetPosition()
        );

        distToTgtMag = Vector3Length(distToTgt);

        if(!tgtLocked) 
        {
            tgtIndex = i;
        }
    }

    if(distToTgtMag <= 4000)
    {
        tgtLocked = true; 

        if(!globalCamera && IsKeyPressed(KEY_E) || globalCamera && IsKeyPressed(KEY_TAB))
        {
            if(tgtIndex < targets.size())
            {
                tgtIndex++;

                if(tgtIndex >= targets.size())
                {
                    tgtIndex = 0;
                }

                currentTarget = targets[tgtIndex];
            }
        }
    }
    else
    {
        currentTarget = nullptr;
        tgtLocked = false;
        return;
    }    
}
