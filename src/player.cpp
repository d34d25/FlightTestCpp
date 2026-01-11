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
    params = GetPlaneParams(Planes::DEBUG_PLANE);

    originalMaxPitchSpeed = params.maxPitchSpeed;
    originalMaxRollSpeed = params.maxRollSpeed;
    originalMaxYawSpeed = params.maxYawSpeed;

    originalPitchResponse = params.pitchAcceleration;
    originalRollResponse = params.rollAcceleration;
    originalYawResponse = params.yawAcceleration;

    pitchUpKey = false;
    pitchDownKey = false;

    rollRightKey = false;
    rollLeftKey = false;

    yawRightKey = false;
    yawLeftKey = false;

    acclerationKey = false;
    brakeKey = false;

    missileKey = false;
    gunKey = false;

    targetSwitchKey = false;

    //body

    body = Body3D(this->params.lateralDragMultiplier,
                  this->params.angularDamping);

    body.transform.translation.x = this->params.position.x;
    body.transform.translation.y = this->params.position.y;
    body.transform.translation.z = this->params.position.z;

    body.transform.scale = {params.scale, params.scale, params.scale};

    thrust = params.idleThrust;
    body.linearVelocity.z = 360;

    // camera
    FOVY = 60.0f;
    camera.fovy = FOVY;
    camera.up.x = 0.0f;
    camera.up.y = -1.0f;
    camera.up.z = 0.0f;

    camera.target = (Vector3){0, 0, 0};
    camera.position = (Vector3){0, 0, 0};

    camera.projection = CAMERA_PERSPECTIVE;

    cameraOffset.x = 0.0f;
    cameraOffset.y = params.camOffsetY;
    cameraOffset.z = params.camOffsetZ;

    cameraAlpha = 10.0f;
    currentFovy = FOVY;
    
    globalCamera = false;
    orbitYaw = 0.0f;
    orbitPitch = 0.0f;
    orbitDistance = -100.0f;

    smoothedOffset.x = 0.0f;
    smoothedOffset.y = 0.0f;
    smoothedOffset.z = 0.0f;

    //input
    pitchInputUp = 0;
    rollInputRight = 0;
    yawInputRight = 0;

    pitchInputDown = 0;
    rollInputLeft = 0;
    yawInputLeft = 0;

    // engine glow
    engineGlow = 0.5f;
    idleEngineGlow = 0.5f;

    maxEngineGlow = 1;

    engineGlowChange = 0.2f;

    //bullets
    bulletPool = BulletPool(60, 1);
    bulletTransform = {};
    bulletTransform.scale = {1.0f, 1.0f, 1.0f};

    fireTimerBullet = 0.0f;
    firerateBullet = 0.1f;

    //missiles
    fireTimerMissileA = 0.0f;
    firerateMissile = 2.0f;

    fireTimerMissileB = 0.0f;

    float missilespeed = MAX_THRUST * 1.25f;

    missilePoolA = MissilePool(6, 4, missilespeed, MISSILE_LOCK_ON_ANGLE, MISSILE_MOBILITY);
    missileTransformA = {};
    missileTransformA.scale = {1.0f, 1.0f, 1.0f};
    missileTransformA.rotation = QuaternionIdentity();

    missilePoolB = MissilePool(6, 4, missilespeed, MISSILE_LOCK_ON_ANGLE, MISSILE_MOBILITY);
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

void Player::UpdatePlayer(float dt)
{
    Vector3 forward = GetWorldForwardVector(body.transform);
    forward = Vector3Normalize(forward);

    if(!globalCamera)
    {
        pitchUpKey = IsKeyDown(KEY_DOWN);
        pitchDownKey = IsKeyDown(KEY_UP);

        rollRightKey = IsKeyDown(KEY_RIGHT);
        rollLeftKey = IsKeyDown(KEY_LEFT);

        yawRightKey = IsKeyDown(KEY_D);
        yawLeftKey = IsKeyDown(KEY_A);

        acclerationKey = IsKeyDown(KEY_W);
        brakeKey = IsKeyDown(KEY_S);

        missileKey = IsKeyPressed(KEY_SPACE);
        gunKey = IsKeyDown(KEY_LEFT_SHIFT);

        targetSwitchKey = IsKeyPressed(KEY_E);
    }
    else
    {
        pitchUpKey = IsKeyDown(KEY_S);
        pitchDownKey = IsKeyDown(KEY_W);

        rollRightKey = IsKeyDown(KEY_D);
        rollLeftKey = IsKeyDown(KEY_A);

        yawRightKey = IsKeyDown(KEY_E);
        yawLeftKey = IsKeyDown(KEY_Q);

        acclerationKey = IsKeyDown(KEY_LEFT_SHIFT);
        brakeKey = IsKeyDown(KEY_LEFT_CONTROL);

        missileKey = IsKeyPressed(KEY_SPACE);
        gunKey = IsKeyDown(KEY_LEFT_ALT);

        targetSwitchKey = IsKeyPressed(KEY_TAB);

        engineGlow = thrust / params.maxThrust;
    }

    debugModeOnKey = IsKeyDown(KEY_F);
    debugModeOffKey = IsKeyDown(KEY_G);

    if(debugModeOnKey)
    {
        debugMode = true;
    }
    else if(debugModeOffKey)
    {
        debugMode = false;
    }

    if(debugMode)
    {
        Vector3 dir = {0,0,0};

        float yawDir = 0;
        float pitchDir = 0;
        float rollDir = 0;

        if(IsKeyDown(KEY_W)) dir.z = 1;
        else if(IsKeyDown(KEY_S)) dir.z = -1;
        else dir.z = 0;

        if(IsKeyDown(KEY_A)) dir.x = 1;
        else if(IsKeyDown(KEY_D)) dir.x = -1;
        else dir.x = 0;

        if(IsKeyDown(KEY_SPACE)) dir.y = 1;
        else if(IsKeyDown(KEY_LEFT_CONTROL)) dir.y = -1;
        else dir.y = 0;

        if(IsKeyDown(KEY_Q)) yawDir = 1;
        else if(IsKeyDown(KEY_E)) yawDir = -1;
        else yawDir = 0;

        if(IsKeyDown(KEY_UP)) pitchDir = 1;
        else if (IsKeyDown(KEY_DOWN)) pitchDir = -1;
        else pitchDir = 0;

        if(IsKeyDown(KEY_LEFT)) rollDir = -1;
        else if (IsKeyDown(KEY_RIGHT)) rollDir = 1;
        else rollDir = 0;

        body.ApplyAlternateLocalForce(dir, 30000);
        body.ApplyAlternateYaw(yawDir * 30);
        body.ApplyAlternatePitch(pitchDir * 30);
        body.ApplyAlternateRoll(rollDir * 30);
    }

    if(!debugMode)
    {
        if(acclerationKey)
        {
            thrust += params.acceleration * dt;

            if(!globalCamera)
            {
                hasInput = true;
                engineGlow += engineGlowChange * dt;
            }
        }
        else if (brakeKey)
        {
            thrust -= params.brake * dt;

            if(!globalCamera)
            {
                engineGlow -= engineGlowChange * dt;
                hasInput = true;
            }
        }
        else
        {
            if(!globalCamera)
            {
                hasInput = false;

                if (engineGlow < idleEngineGlow)
                {
                    engineGlow += engineGlowChange * dt;
                }
                else if (engineGlow > idleEngineGlow)
                {
                    engineGlow -= engineGlowChange * dt;
                }
                else
                {
                    engineGlow = idleEngineGlow;
                }
            }
        }

        engineGlow = Clamp(engineGlow, 0, maxEngineGlow);

        if(pitchUpKey)
        {
            pitchInputDown = 0;
            pitchInputUp -= params.pitchAcceleration * dt;
        }
        else if (pitchDownKey)
        {
            pitchInputUp = 0;
            pitchInputDown += params.pitchAcceleration * dt;
        }
        else
        {
            pitchInputDown = 0;
            pitchInputUp = 0;
        }

        if(rollRightKey)
        {
            rollInputLeft = 0;
            rollInputRight += params.rollAcceleration * dt;
        }
        else if (rollLeftKey)
        {
            rollInputRight = 0;
            rollInputLeft -= params.rollAcceleration * dt;
        }
        else
        {
            rollInputLeft = 0;
            rollInputRight = 0;
        }

        if(yawRightKey)
        {
            yawInputLeft = 0;
            yawInputRight -= params.yawAcceleration * dt;
        }
        else if (yawLeftKey)
        {
            yawInputRight = 0;
            yawInputLeft += params.yawAcceleration * dt;
        }
        else
        {
            yawInputLeft = 0;
            yawInputRight = 0;
        }

        pitchInputUp = Clamp(pitchInputUp, -params.pitchPower, 0);
        rollInputRight = Clamp(rollInputRight, 0, params.rollPower);
        yawInputRight = Clamp(yawInputRight, -params.yawPower, 0);

        pitchInputDown = Clamp(pitchInputDown, 0, params.pitchPower);
        rollInputLeft = Clamp(rollInputLeft, -params.rollPower, 0);
        yawInputLeft = Clamp(yawInputLeft, 0, params.yawPower);

        /*std::cout<<"pitch up : "<<pitchInputUp<<"\n";
        std::cout<<"roll right : "<<rollInputRight<<"\n";
        std::cout<<"yaw right : "<<yawInputRight<<"\n";

        std::cout<<"pitch down : "<<pitchInputDown<<"\n";
        std::cout<<"roll left : "<<rollInputLeft<<"\n";
        std::cout<<"yaw left : "<<yawInputLeft<<"\n";*/

        float finalPtich = pitchInputUp + pitchInputDown;
        float finalRoll = rollInputLeft + rollInputRight;
        float finalYaw = yawInputLeft + yawInputRight;

        body.ApplyPitch(finalPtich);
        body.ApplyRoll(finalRoll);
        body.ApplyYaw(finalYaw);

        // plane

        body.angularVelocity.x = Clamp(body.angularVelocity.x, -params.maxPitchSpeed, params.maxPitchSpeed); 
        body.angularVelocity.z = Clamp(body.angularVelocity.z, -params.maxRollSpeed, params.maxRollSpeed); 
        body.angularVelocity.y = Clamp(body.angularVelocity.y, -params.maxYawSpeed, params.maxYawSpeed);

        /*std::cout<<"angular vel x: "<<body.angularVelocity.x<<"\n";
        std::cout<<"angular vel z: "<<body.angularVelocity.z<<"\n";
        std::cout<<"angular vel y: "<<body.angularVelocity.y<<"\n";*/

        if (!hasInput && returnToIdle)
        {
            if (thrust <= params.idleThrust)
            {
                thrust += params.returnSpeedLow * dt;
                if (thrust >= params.idleThrust) thrust = params.idleThrust;
            }
            else
            {
                thrust -= params.returnSpeedHigh * dt;
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

        float forwardSpeed = Vector3DotProduct(body.GetTrueLinearVelocity(), forward);

        if (dotFU > 0.1)
        {
            if (forwardSpeed > 0.0f) body.ApplyAlternateLocalForce(backDir, FAKE_GRAVITY_FORCE * dotFU);
        }
        else if (dotFU < -0.1)
        {
            body.ApplyAlternateLocalForce(backDir, FAKE_GRAVITY_FORCE * dotFU);
        }

        // fake banking

        Vector3 right = GetWorldRightVector(body.transform);
        right = Vector3Normalize(right);

        float rDot = Vector3DotProduct(upDir, right);

        body.ApplyAlternateYaw(MAX_BANK_TORQUE_YAW * -rDot);

        // upside down case

        Vector3 up = GetWorldUpVector(body.transform);
        up = Vector3Normalize(up);

        float uDot = Vector3DotProduct(upDir, up);

        if (uDot <= -0.1)
        {
            body.ApplyAlternatePitch(MAX_BANK_TORQUE_PITCH * uDot);
        }

        // fake stall

        float speed = GetTrueLinearSpeed();

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

        float mobilityFactor = 1.0f;

        if (speed <= params.mobilityLooseStartSpeed)
        {
            mobilityFactor = 1 - (1 - params.mobilityProportionLow) * (speed - params.mobilityLooseStartSpeed) / (params.stallSpeed - params.mobilityLooseStartSpeed);
            mobilityFactor = Clamp(mobilityFactor, params.mobilityProportionLow, 1.0f);
        }
        else
        {
            mobilityFactor = 1 - (1 - params.mobilityProportionHigh) * (speed - params.mobilityLooseStartSpeed) / (GetMaxSpeed() - params.mobilityLooseStartSpeed);
            mobilityFactor = Clamp(mobilityFactor, params.mobilityProportionHigh, 1.0f);
        }

        params.maxPitchSpeed = originalMaxPitchSpeed * mobilityFactor;
        params.maxRollSpeed = originalMaxRollSpeed * mobilityFactor;
        params.maxYawSpeed = originalMaxYawSpeed * mobilityFactor;

        float responseFactor = 1.0f;
        
        if (speed <= params.mobilityLooseStartSpeed)
        {
            responseFactor = 1 - (1 - params.responsivenessProportionLow) * (speed - params.mobilityLooseStartSpeed) / (params.stallSpeed - params.mobilityLooseStartSpeed);
            responseFactor = Clamp(responseFactor, params.responsivenessProportionLow, 1.0f);
        }
        else
        {
            responseFactor = 1 - (1 - params.responsivenessProportionHigh) * (speed - params.mobilityLooseStartSpeed) / (GetMaxSpeed() - params.mobilityLooseStartSpeed);
            responseFactor = Clamp(responseFactor, 1.0f, params.responsivenessProportionHigh);
        }

        params.pitchAcceleration = originalPitchResponse * responseFactor;
        params.rollAcceleration = originalRollResponse * responseFactor;
        params.yawAcceleration =  originalYawResponse * responseFactor;


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

        Vector3 axisOfRotation = Vector3CrossProduct(forward, downDir);

        axisOfRotation = Vector3Normalize(axisOfRotation);

        if (stalling && fDot < fDotTarget)
        {
            body.worldAngularTorque = STALL_TORQUE_SPEED;
        }

        body.ApplyAlternateWorldTorque(axisOfRotation, dt); //stall
    }

    body.SingleBodyUpdate(dt);

    //bulletPool.UpdateBullets(dt);

    //missilePoolA.UpdateMissiles(dt);
    //missilePoolB.UpdateMissiles(dt);
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

        float alpha = cameraAlpha * dt;

        float fovyFactor = 1.0f;

        float speed = GetTrueLinearSpeed();

        float minFovy = 0.85f;
        float maxFovy = 1.15f;

        if(speed <= GetIdleSpeed())
        {
            fovyFactor = 1 - (1 - minFovy) * (speed - GetIdleSpeed()) / (-GetIdleSpeed());
            fovyFactor = Clamp(fovyFactor, minFovy, 1.0f);
        }
        else
        {
            fovyFactor = 1 - (1 - maxFovy) * (speed - GetIdleSpeed()) / (GetMaxSpeed() * 0.75f - GetIdleSpeed());
            fovyFactor = Clamp(fovyFactor, 1.0f, maxFovy);
        }

        camera.fovy = FOVY * fovyFactor;

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

    if (gunKey && !debugMode)
    {
        while (fireTimerBullet <= 0.0f)
        {
            FollowTransform(&bulletTransform, GetTransform(), params.gunPos);

            Vector3 forward = GetWorldForwardVector(body.transform);
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
    Vector3* tgtPos = nullptr;

    if(tgtLocked)
    {
        if(currentTarget)
        {
            locked = true;
            tgtPos = &currentTarget->body.transform.translation;
        }
        else
        {
            tgtPos = nullptr;
        }
    }

    if (fireTimerMissileA > 0.0f) fireTimerMissileA -= dt;
    if (fireTimerMissileB > 0.0f) fireTimerMissileB -= dt;

    if(missileKey && !debugMode)
    {
        Vector3 forward = GetWorldForwardVector(body.transform);
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
    float closestDist = INFINITY;
    int bestTarget = -1;

    float frontAngle = 0.75f;
    float maxDist = 4000.0f;

    Vector3 forward = GetWorldForwardVector(body.transform);

    for (int i = 0; i < targets.size(); i++)
    {
        Target* tgt = targets[i];

        if (tgt) 
        {
            Vector3 diff = Vector3Subtract(tgt->body.transform.translation, GetPosition());
            float dist = Vector3Length(diff);

            if (dist > maxDist) continue;

            Vector3 dir = Vector3Normalize(diff);
            float angle = Vector3DotProduct(forward, dir);

            if (angle < frontAngle) continue;

            if (dist < closestDist)
            {
                closestDist = dist;
                bestTarget = i;
            }
        }
    }

    if (bestTarget < 0)
    {
        currentTarget = nullptr;
        tgtLocked = false;
        return;
    }

    if (!tgtLocked)
    {
        tgtLocked = true;
        tgtIndex = bestTarget;
        currentTarget = targets[tgtIndex];
    }

    if (targetSwitchKey)
    {
        int startIndex = tgtIndex;

        do
        {
            tgtIndex = (tgtIndex + 1) % targets.size();

            Target* tgt = targets[tgtIndex];

            if (tgt) 
            {
                Vector3 diff = Vector3Subtract(tgt->body.transform.translation, GetPosition());
                float dist = Vector3Length(diff);
                Vector3 dir = Vector3Normalize(diff);

                float angle = Vector3DotProduct(forward, dir);

                if (dist < maxDist && angle > frontAngle)
                {
                    currentTarget = targets[tgtIndex];
                    break;
                }
            }

        } while (tgtIndex != startIndex);
    }
}

