#include "player.h"
#include <cmath>
#include <iostream>


//merge plane and player into one class
//because the enemy AI won't use the 
//flight model or any plane phyisics
Player::Player(float scale)
{
    params.position.x = 0.0f;
    params.position.y = 400.0f;
    params.position.z = -700.0f;

    params.angularDamping.x = DEFAULT_ANGULAR_DAMPING;
    params.angularDamping.y = DEFAULT_ANGULAR_DAMPING;
    params.angularDamping.z = DEFAULT_ANGULAR_DAMPING;

    params.maxThrust = 350000.0f;
    params.idleThrust = DEFAULT_IDLE_THRUST;

    params.returnSpeedHigh = DEFAULT_RETURN_SPEED_HIGH * 1.15f;
    params.returnSpeedLow = DEFAULT_RETURN_SPEED_LOW;

    params.acceleration = 21000.0f;
    params.brake = 18000.0f;

    params.pitchPower = 70.0f;
    params.rollPower = 190.0f;
    params.yawPower = 25.0f;
    
    params.maxPitchSpeed = 1.0f;
    params.maxRollSpeed = 1.8f;
    params.maxYawSpeed = 0.23f;

    params.hitboxWidth =  20;
    params.hitboxLength = 23;
    params.hitboxHeight = 2;

    thrust = 0.0f;

    originalMaxPitchSpeed = params.maxPitchSpeed;
    originalMaxRollSpeed = params.maxRollSpeed;
    originalMaxYawSpeed = params.maxYawSpeed;

    body = Body3D(this->params.linearDamping,
    this->params.angularDamping);

    body.transform.translation.x = this->params.position.x;
    body.transform.translation.y = this->params.position.y;
    body.transform.translation.z = this->params.position.z;

    //params.stallSpeed = 0;

    params.modelPath = "assets/sf15b.obj";

    body.transform.scale.x = scale;
    body.transform.scale.y = scale;
    body.transform.scale.z = scale;

    thrust = params.idleThrust * 1.25f;

    //camera
    camera.fovy = 60.0f;
    camera.up.x = 0.0f;
    camera.up.y = -1.0f;
    camera.up.z = 0.0f;

    camera.target = (Vector3){0,0,0};
    camera.position = (Vector3){0,0,0};

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

    //engine glow
    engineGlow = 0.5f;
    idleEngineGlow = 0.5f;

    maxEngineGlow = 1;

    engineGlowChange = 0.2f;

    bulletPool = BulletPool(60,6,0.05f);
}

void Player::UpdatePlayer(float dt, int iterations)
{
    float fdt = dt;
    if(iterations > 0) fdt /= iterations;

    if(!globalCamera)
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

        if (IsKeyDown(KEY_D))  yawInput = -1;
        else if (IsKeyDown(KEY_A)) yawInput  = 1;
        else yawInput = 0;

        if (IsKeyDown(KEY_RIGHT))  rollInput = 1;
        else if (IsKeyDown(KEY_LEFT)) rollInput  = -1;
        else rollInput = 0;

        if (IsKeyDown(KEY_UP))  pitchInput = 1;
        else if (IsKeyDown(KEY_DOWN)) pitchInput  = -1;
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

        if (IsKeyDown(KEY_E))  yawInput = -1;
        else if (IsKeyDown(KEY_Q)) yawInput  = 1;
        else yawInput = 0;

        if (IsKeyDown(KEY_D))  rollInput = 1;
        else if (IsKeyDown(KEY_A)) rollInput  = -1;
        else rollInput = 0;

        if (IsKeyDown(KEY_W))  pitchInput = 1;
        else if (IsKeyDown(KEY_S)) pitchInput  = -1;
        else pitchInput = 0;
    }

    body.ApplyPitch(pitchInput * params.pitchPower);
    body.ApplyRoll(rollInput * params.rollPower);
    body.ApplyYaw(yawInput * params.yawPower);

    //plane
    if (body.angularVelocity.x >= params.maxPitchSpeed) body.angularVelocity.x = params.maxPitchSpeed;
    else if (body.angularVelocity.x <= -params.maxPitchSpeed) body.angularVelocity.x = -params.maxPitchSpeed;
 
    if (body.angularVelocity.y >= params.maxYawSpeed) body.angularVelocity.y = params.maxYawSpeed;
    else if (body.angularVelocity.y <= -params.maxYawSpeed) body.angularVelocity.y = -params.maxYawSpeed;
   
    if (body.angularVelocity.z >= params.maxRollSpeed) body.angularVelocity.z = params.maxRollSpeed;
    else if (body.angularVelocity.z <= -params.maxRollSpeed) body.angularVelocity.z = -params.maxRollSpeed;

    if (!hasInput && returnToIdle)
    {
        if(thrust <= params.idleThrust)
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

    //move this outside the function
    Vector3 forwardDir;
    forwardDir.x = 0.0f, forwardDir.y = 0.0f, forwardDir.z = 1.0f;

    Vector3 upDir;
    upDir.x = 0.0f, upDir.y = 1.0f, upDir.z = 0.0f;

    Vector3 backDir;
    backDir.x = 0.0f, backDir.y = 0.0f, backDir.z = -1.0f;

    Vector3 rightDir;
    rightDir.x = 1.0f, rightDir.y = 0.0f, rightDir.z = 0.0f;

    Vector3 downDir;
    downDir.x = 0.0f, downDir.y = -1.0f, downDir.z = 0.0f;


    body.ApplyLocalForce(forwardDir, thrust);

    //drag / fake gravity

    Vector3 forward = GetLocalForwardVector(body.transform);
    forward = Vector3Normalize(forward);

    float dotFU = Vector3DotProduct(forward, upDir);

    float fakeGravity = 15000;

    float forwardSpeed = Vector3DotProduct(body.linearVelocity, forward);

    if (dotFU > 0.1)
    {
        if(forwardSpeed > 0.0f) body.ApplyLocalForce(backDir, fakeGravity * dotFU); 
    }
    else if (dotFU < -0.1)
    {
        body.ApplyLocalForce(backDir, fakeGravity * dotFU);
    }

    //fake banking

    float maxBankTorque = 5.0f;

    Vector3 right = GetLocalRightVector(body.transform);
    right = Vector3Normalize(right);

    float rDot = Vector3DotProduct(upDir, right);

    body.ApplyYaw(maxBankTorque * - rDot);
    
    //upside down case

    Vector3 up = GetLocalUpVector(body.transform);
    up = Vector3Normalize(up);

    float uDot = Vector3DotProduct(upDir, up);
    
    if (uDot <= -0.1)
    {
        body.ApplyPitch(maxBankTorque * uDot);
    }
    
    //fake stall

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

        body.ApplyForce(downDir, 4000);
    }

    Vector3 axisOfRotation = Vector3CrossProduct(forward, downDir);

    axisOfRotation = Vector3Normalize(axisOfRotation);

    if(stalling && fDot < fDotTarget)
    {
        body.stallAngularTorque = params.stallTorqueSpeed;
    }

    body.ApplyWorldTorque(axisOfRotation, fdt);

    body.UpdateBody(dt, iterations);

    Fire(fdt);
    bulletPool.UpdateBullets(fdt);
}

void Player::UpdateCamera(float dt)
{
    Vector3 rotatedOffset = Vector3RotateByQuaternion(cameraOffset, GetOrientation());

    if (IsKeyPressed(KEY_ONE)) globalCamera = !globalCamera;
    
    if(!globalCamera)
    {
        returnToIdle = true;

        float alpha = 10.0f * dt;

        smoothedOffset.x += alpha * (rotatedOffset.x - smoothedOffset.x);
        smoothedOffset.y += alpha * (rotatedOffset.y - smoothedOffset.y);
        smoothedOffset.z += alpha * (rotatedOffset.z - smoothedOffset.z);
        
        camera.position.x = GetPosition().x + smoothedOffset.x;
        camera.position.y = GetPosition().y + smoothedOffset.y;
        camera.position.z = GetPosition().z + smoothedOffset.z;

        Vector3 localForward = {0.0f,0.14f,1.0f};

        Vector3 forward = Vector3RotateByQuaternion(localForward, GetOrientation());

        int lookAhead = 200;

        camera.target.x = GetPosition().x + forward.x * lookAhead;
        camera.target.y = GetPosition().y + forward.y * lookAhead;
        camera.target.z = GetPosition().z + forward.z * lookAhead;

        Vector3 localUp = {0.0f,1.0f,0.0f};

        Vector3 upWorld = upWorld = Vector3RotateByQuaternion(localUp,GetOrientation());

        camera.up.x = upWorld.x;
        camera.up.y = upWorld.y;
        camera.up.z = upWorld.z;
    }
    else
    {
        returnToIdle = false;

        hasInput = true;

        HideCursor();
        
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;

        Vector2 delta = GetMouseDelta();

        float wheel = GetMouseWheelMove();

        orbitDistance += wheel * 2.0f;
        orbitDistance = Clamp(orbitDistance, -200.0f,-10.0f);

        orbitYaw -= delta.x * 0.4f * dt;
        orbitPitch -= delta.y * 0.4f * dt;

        orbitPitch = Clamp(orbitPitch, -1.4f,1.4f);

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


void Player::Fire(float dt)
{
    int bulletspeed = 800; //800

    fireTimer += dt;

    //make this an attribute of the plane
    //call it gun position
    Transform bulletTransform = {};
    FollowTransform(&bulletTransform, GetTransform(),{-2.75f,0.9f,4.0f});
    bulletTransform.scale = {1.0f,1.0f,1.0f};

    bool fireKey = (!globalCamera && IsKeyDown(KEY_LEFT_SHIFT)) || (globalCamera && IsKeyDown(KEY_SPACE));

    Vector3 forward = GetLocalForwardVector(body.transform);
    float forwardSpeed = Vector3DotProduct(body.linearVelocity, forward);

    if(fireKey)
    {
        Vector3 bulletVelocity = Vector3Add(body.linearVelocity, Vector3Scale(forward, bulletspeed));

        while (fireTimer >= firerate)
        {
            bulletPool.FireBullet(bulletTransform, bulletVelocity);
            fireTimer -= firerate;
        }
    }
    else
    {
        fireTimer = 0.0f;
    }
    
}
