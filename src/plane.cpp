#include "plane.h"
#include "simpleTransform.h"
#include <iostream>

const float DEFAULT_STALL_SPEED = 140.0f;
const float DEFAULT_RECOVERY_SPEED = 170.0f;
const float DEFAULT_MOBILITY_LOOSE_START_SPEED = 250.0f;


const float MAX_THRUST = 175000 * FORWARD_DRAG_MULTIPLIER; //1700 units/s aprox

PlaneParams GetPlaneParams(Planes plane)
{
    PlaneParams params;

    switch (plane)
    {
    case Planes::SF15:

        params.scale = 2.0f;

        params.lateralDragMultiplier = 4.5f;
        params.angularDamping = {3,3,3};

        params.maxThrust = 175000;
        params.idleThrust = 37500;

        params.acceleration = 10500;
        params.brake = 9500;

        params.returnSpeedHigh = 4000;
        params. returnSpeedLow = 2500;

        params.pitchPower = 20.0f;
        params.rollPower = 60.0f;
        params.yawPower = 10.0f;

        params.maxPitchSpeed = 0.8f;
        params.maxRollSpeed = 1.6f;
        params.maxYawSpeed = 0.2f;

        params.hitboxWidth = 20;
        params.hitboxLength = 23;
        params.hitboxHeight = 2;

        params.modelPath = "assets/sf15b.obj";

        params.stallSpeed = DEFAULT_STALL_SPEED;
        params.recoverySpeed = DEFAULT_RECOVERY_SPEED;
        params.mobilityLooseStartSpeed = DEFAULT_MOBILITY_LOOSE_START_SPEED;

        params.proportionHigh = 0.8f;
        params.proportionLow = 0.6f;

        break;
    
    default:
        break;
    }

    params.maxThrust *= FORWARD_DRAG_MULTIPLIER;
    params.idleThrust *= FORWARD_DRAG_MULTIPLIER;

    params.returnSpeedHigh *= FORWARD_DRAG_MULTIPLIER;
    params.returnSpeedLow *= FORWARD_DRAG_MULTIPLIER;

    params.acceleration *= FORWARD_DRAG_MULTIPLIER;
    params.brake *= FORWARD_DRAG_MULTIPLIER;
    
    params.position = {0,400,-700};

    return params;
}
