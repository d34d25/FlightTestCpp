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

    originalPitchResponsiveness = params.pitchResponsiveness;
    originalRollResponsiveness = params.rollResponsiveness;
    originalYawResponsiveness = params.yawResponsiveness;

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
    cameraOffset.y = 5.0f;
    cameraOffset.z = -39.0f;

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
    currentTarget.reset();

    tgtIndex = 0;
    tgtLocked = false;
}

void Player::UpdatePlayer(float dt, int iterations)
{
    float fdt = dt;
    fdt /= iterations;

    Vector3 forward = GetWorldForwardVector(body.transform);
    forward = Vector3Normalize(forward);

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

        if (IsKeyDown(KEY_D)) 
        {   
            yawInputLeft = 0;
            yawInputRight += -params.yawResponsiveness * fdt;
        }   
        else if (IsKeyDown(KEY_A))
        {
            yawInputRight = 0;
            yawInputLeft += params.yawResponsiveness * fdt;
        } 
        else
        {
            yawInputLeft = 0;
            yawInputRight = 0;
        } 

        if (IsKeyDown(KEY_RIGHT))
        {
            rollInputLeft = 0;
            rollInputRight += params.rollResponsiveness * fdt;
        } 
        else if (IsKeyDown(KEY_LEFT))
        {
            rollInputRight = 0;
            rollInputLeft += -params.rollResponsiveness * fdt;
        } 
        else
        {
            rollInputLeft = 0;
            rollInputRight = 0;
        }

        if (IsKeyDown(KEY_DOWN))
        {
            pitchInputDown = 0;
            pitchInputUp += -params.pitchResponsiveness * fdt;
        } 
        else if (IsKeyDown(KEY_UP)) 
        {
            pitchInputUp = 0;
            pitchInputDown += params.pitchResponsiveness * fdt;
        } 
        else
        {
            pitchInputDown = 0;
            pitchInputUp = 0;
        } 
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

        if (IsKeyDown(KEY_E))
        {
            yawInputLeft = 0;
            yawInputRight += -params.yawResponsiveness * fdt;
        } 
        else if (IsKeyDown(KEY_Q))
        {
            yawInputRight = 0;
            yawInputLeft += params.yawResponsiveness * fdt;
        }
        else
        {
            yawInputLeft = 0;
            yawInputRight = 0;
        } 

        if (IsKeyDown(KEY_D))
        {
            rollInputLeft = 0;
            rollInputRight += params.rollResponsiveness * fdt;
        }
        else if (IsKeyDown(KEY_A))
        {
            rollInputRight = 0;
            rollInputLeft += -params.rollResponsiveness * fdt;
        }
        else
        {
            rollInputLeft = 0;
            rollInputRight = 0;
        }

        if (IsKeyDown(KEY_S))
        {
            pitchInputDown = 0;
            pitchInputUp += -params.pitchResponsiveness * fdt;
        }
        else if (IsKeyDown(KEY_W))
        {
            pitchInputUp = 0;
            pitchInputDown += params.pitchResponsiveness * fdt;
        }
        else
        {
            pitchInputDown = 0;
            pitchInputUp = 0;
        }
    }

    float maxResponsiveness = 1000.0f;

    Clamp(pitchInputUp, -maxResponsiveness, 0);
    Clamp(rollInputRight, 0, maxResponsiveness);
    Clamp(yawInputRight, -maxResponsiveness, 0);

    Clamp(pitchInputDown, 0, maxResponsiveness);
    Clamp(rollInputLeft, -maxResponsiveness, 0);
    Clamp(yawInputLeft, 0, maxResponsiveness);

    body.ApplyPitch(pitchInputUp * body.angularDamping.x);
    body.ApplyRoll(rollInputRight * body.angularDamping.z);
    body.ApplyYaw(yawInputRight * body.angularDamping.y);

    body.ApplyPitch(pitchInputDown * body.angularDamping.x);
    body.ApplyRoll(rollInputLeft * body.angularDamping.z);
    body.ApplyYaw(yawInputLeft * body.angularDamping.y);

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

    float forwardSpeed = Vector3DotProduct(body.GetTrueVelocity(), forward);

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

    float speed = GetTrueSpeed();

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

    float responsivenessFactor = 1.0f;

    if (speed <= params.mobilityLooseStartSpeed)
    {
        responsivenessFactor = 1 - (1 - params.responsivenessProportionLow) * (speed - params.mobilityLooseStartSpeed) / (params.stallSpeed - params.mobilityLooseStartSpeed);
        responsivenessFactor = Clamp(responsivenessFactor, params.responsivenessProportionLow, 1.0f);
    }
    else
    {
        responsivenessFactor = 1 - (1 - params.responsivenessProportionHigh) * (speed - params.mobilityLooseStartSpeed) / (GetMaxSpeed() - params.mobilityLooseStartSpeed);
        responsivenessFactor = Clamp(responsivenessFactor, 1.0f, params.responsivenessProportionHigh);
    }

    params.pitchResponsiveness = originalPitchResponsiveness * responsivenessFactor;
    params.rollResponsiveness = originalRollResponsiveness * responsivenessFactor;
    params.yawResponsiveness = originalYawResponsiveness * responsivenessFactor;

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

    body.ApplyAlternateWorldTorque(axisOfRotation, fdt); //stall

    //body.AlternateUpdateBody(dt, iterations); //external forces

    //body.UpdateBody(dt, iterations); //player input

    body.SingleBodyUpdate(dt,iterations);

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

        float alpha = cameraAlpha * dt;

        float fovyFactor = 1.0f;

        float speed = GetTrueSpeed();

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

    bool fireKey = (!globalCamera && IsKeyDown(KEY_LEFT_SHIFT)) || (globalCamera && IsKeyDown(KEY_SPACE));

    if (fireKey && fireTimerBullet <= 0.0f)
    {
        while (fireTimerBullet <= 0.0f)
        {
            FollowTransform(&bulletTransform, GetTransform(), {-2.75f, 0.9f, 10.0f});

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
        if(auto t = currentTarget.lock())
        {
            locked = true;
            tgtPos = &t->body.transform.translation;
        }
        else
        {
            tgtPos = nullptr;
        }
    }

    if (fireTimerMissileA > 0.0f) fireTimerMissileA -= dt;
    if (fireTimerMissileB > 0.0f) fireTimerMissileB -= dt;

    bool fireKey = (!globalCamera && IsKeyPressed(KEY_SPACE)) || (globalCamera && IsKeyPressed(KEY_LEFT_ALT));

    if(fireKey)
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
        if (auto t = targets[i].lock()) 
        {
            Vector3 diff = Vector3Subtract(t->body.transform.translation, GetPosition());
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
        currentTarget.reset();
        tgtLocked = false;
        return;
    }

    if (!tgtLocked)
    {
        tgtLocked = true;
        tgtIndex = bestTarget;
        currentTarget = targets[tgtIndex];
    }

    bool switchPressed = (!globalCamera && IsKeyPressed(KEY_E)) || ( globalCamera && IsKeyPressed(KEY_TAB));

    if (switchPressed)
    {
        int startIndex = tgtIndex;

        do
        {
            tgtIndex = (tgtIndex + 1) % targets.size();

            if (auto t = targets[tgtIndex].lock()) 
            {
                Vector3 diff = Vector3Subtract(t->body.transform.translation, GetPosition());
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

