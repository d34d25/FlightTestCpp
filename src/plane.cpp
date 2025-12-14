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

    float angDamp = 3.0f;
    params.skipMaterialIndex = 2;

    float responsive = 120;

    switch (plane)
    {
    case Planes::DEBUG_PLANE:

        params.scale = 2.0f;

        params.hitboxWidth = 2; //20
        params.hitboxLength = 2; //23
        params.hitboxHeight = 2;

        angDamp = 3.0f;
        params.angularDamping = {
            angDamp,
            angDamp,
            angDamp
        }; //3.0f

        params.lateralDragMultiplier = 200.0f; //200.0f
        
        params.maxThrust = 175000 *30;
        params.idleThrust = 30000;

        params.acceleration = 10500 *900;//  / 2.0f;
        params.brake = 9500 / 2.0f;

        params.returnSpeedHigh = 4000 / 2.0f;
        params.returnSpeedLow = 2500 / 2.0f;

        params.mobilityProportionHigh = 0.7f; //0.8f;
        params.mobilityProportionLow = 0.4f; //0.6f;

        params.responsivenessProportionHigh = 4.0f;
        params.responsivenessProportionLow = 0.4f;

        params.stallSpeed = DEFAULT_STALL_SPEED;
        params.recoverySpeed = DEFAULT_RECOVERY_SPEED;
        params.mobilityLooseStartSpeed = DEFAULT_MOBILITY_LOOSE_START_SPEED;
       
        params.maxPitchSpeed = 0.65f;
        params.maxRollSpeed = 1.2f;
        params.maxYawSpeed = 0.1f;

        responsive = 100;

        params.pitchResponsiveness = responsive * 0.75f;
        params.rollResponsiveness = responsive * 3.0f;
        params.yawResponsiveness = responsive * 0.5f;

        params.modelPath = "assets/sf15b.obj";
        params.skipMaterialIndex = 7;

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
