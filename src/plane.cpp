#include "plane.h"
#include "simpleTransform.h"
#include <iostream>

const float DEFAULT_LINEAR_DAMPING = 2.0f;
const float DEFAULT_ANGULAR_DAMPING = 3.0f;

const float DEFAULT_IDLE_THRUST = 75000;

const float DEFAULT_STALL_SPEED = 130.0f;
const float DEFAULT_RECOVERY_SPEED = 140.0f;
const float DEFAULT_MOBILITY_LOOSE_START_SPEED = 250.0f;

const float DEFAULT_STALL_TORQUE_SPEED = 30.0f;
const float DEFAULT_STALL_TORQUE_STOP_SPEED = 30.0f;

const float DEFAULT_MAX_STALL_TORQUE = 500.0f;

const float DEFAULT_PROPORTION_LOW = 0.6f;
const float DEFAULT_PROPORTION_HIGH = 0.8f;

const float DEFAULT_RETURN_SPEED_HIGH = 10000.0f * 0.8f;
const float DEFAULT_RETURN_SPEED_LOW = 10000.0f * 0.5f;


Plane::Plane(PlaneParams params)
{
    thrust = 0.0f;
    this->params = params;

    originalMaxPitchSpeed = this->params.maxPitchSpeed;
    originalMaxRollSpeed = this->params.maxRollSpeed;
    originalMaxYawSpeed = this->params.maxYawSpeed;

    body = Body3D(this->params.linearDamping,
    this->params.angularDamping);

    body.transform.translation.x = this->params.position.x;
    body.transform.translation.y = this->params.position.y;
    body.transform.translation.z = this->params.position.z;
}

void Plane::UpdatePlane(float dt, int iterations)
{
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

    float fDotTarget = 0.9f;

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

    if (stalling && fDot < fDotTarget)
    {
        stallTorque += params.stallTorqueSpeed;
    }
    else
    {
        if (stallTorque > 0.0f)
        {
            stallTorque -= params.stallTorqueStopSpeed;
        }
        else if (stallTorque <= 0.0f)
        {
            stallTorque = 0.0f;
        }
    }

    if (stallTorque > params.maxStallTorque)
    {
        stallTorque = params.maxStallTorque;
    }

    body.ApplyWorldTorque(angleSpeed, stallTorque, axisOfRotation, dt, iterations);

    body.UpdateBody(dt, iterations);
}