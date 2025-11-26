#include "player.h"
#include <cmath>

Player::Player(float scale)
{
    PlaneParams params = PlaneParams();
    params.position.x = 0.0f;
    params.position.y = 400.0f;
    params.position.z = -700.0f;

    params.angularDamping.x = DEFAULT_ANGULAR_DAMPING;
    params.angularDamping.y = DEFAULT_ANGULAR_DAMPING;
    params.angularDamping.z = DEFAULT_ANGULAR_DAMPING;

    params.maxThrust = 350000.0f;
    params.idleThrust = DEFAULT_IDLE_THRUST;

    params.returnSpeedHigh = DEFAULT_RETURN_SPEED_HIGH * 1.25f;
    params.returnSpeedLow = DEFAULT_RETURN_SPEED_LOW;

    params.acceleration = 21000.0f;
    params.brake = 18000.0f;

    params.pitchPower = 70.0f;
    params.rollPower = 190.0f;
    params.yawPower = 25.0f;
    
    params.maxPitchSpeed = 1.0f;
    params.maxRollSpeed = 1.8f;
    params.maxYawSpeed = 0.23f;

    params.hitboxWidth = 20;
    params.hitboxLength = 30;
    params.hitboxHeight = 2;

    params.modelPath = "assets/sf15b.obj";

    plane = Plane(params);

    plane.body.transform.scale.x = scale;
    plane.body.transform.scale.y = scale;
    plane.body.transform.scale.z = scale;

    plane.thrust = plane.params.idleThrust * 1.25f;

    //camera
    camera.fovy = 60.0f;
    camera.up.x = 0.0f;
    camera.up.y = -1.0f;
    camera.up.z = 0.0f;

    camera.target = (Vector3){0,0,0};
    camera.position = (Vector3){0,0,0};

    camera.projection = CAMERA_PERSPECTIVE;

    cameraOffset.x = 0.0f;
    cameraOffset.y = 3.0f;
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

    engineGlowChange = 0.005f;
}

void Player::UpdatePlayer(float dt, int iterations)
{
    if(!globalCamera)
    {
        if (IsKeyDown(KEY_W))
        {
            plane.thrust += plane.params.acceleration * dt;

            engineGlow += engineGlowChange;

            plane.hasInput = true;
        }
        else if (IsKeyDown(KEY_S))
        {
            plane.thrust -= plane.params.brake * dt;

            engineGlow -= engineGlowChange;

            plane.hasInput = true;
        }
        else
        {
            plane.hasInput = false;

            if (engineGlow < idleEngineGlow)
            {
                engineGlow += engineGlowChange;
            } 
            else if (engineGlow > idleEngineGlow)
            {
                engineGlow -= engineGlowChange;
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
        plane.returnToIdle = false;

        engineGlow = plane.thrust / plane.params.maxThrust;
        
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            plane.thrust += plane.params.acceleration * dt;
        }
        else if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            plane.thrust -= plane.params.brake * dt;
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

    plane.body.ApplyPitch(pitchInput * plane.params.pitchPower);
    plane.body.ApplyRoll(rollInput * plane.params.rollPower);
    plane.body.ApplyYaw(yawInput * plane.params.yawPower);

    plane.UpdatePlane(dt,iterations);
}

void Player::UpdateCamera(float dt)
{
    Vector3 rotatedOffset = Vector3RotateByQuaternion(cameraOffset, GetOrientation());

    if (IsKeyPressed(KEY_ONE))
    {
        globalCamera = !globalCamera;
    }

    if(!globalCamera)
    {
        plane.returnToIdle = true;

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
        plane.returnToIdle = false;

        plane.hasInput = true;

        HideCursor();
        
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;

        Vector2 delta = GetMouseDelta();

        float wheel = GetMouseWheelMove();

        orbitDistance += wheel * 2.0f;
        orbitDistance = Clamp(orbitDistance, -100.0f,-10.0f);

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
