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

    float response = 120;

    switch (plane)
    {
    case Planes::DEBUG_PLANE:

        params.scale = 1.0f;

        params.hitboxWidth = 15; //20
        params.hitboxLength = 24; //23
        params.hitboxHeight = 2;

        params.colliderPos = {0,0,2};

        params.camOffsetY = 5.0f;
        params.camOffsetZ = -30.0f;

        params.gunPos = {-2.45f, 0.9f, 7.0f};

        angDamp = 3.0f;
        params.angularDamping = {
            angDamp,
            angDamp,
            angDamp
        }; //3.0f

        params.lateralDragMultiplier = 200.0f; //200.0f
        
        params.maxThrust = 175000;// * 30;
        params.idleThrust = 30000; 

        params.acceleration = 10500;// * 900;//  / 2.0f;
        params.brake = 9500;

        params.returnSpeedHigh = 4000;
        params.returnSpeedLow = 2500;

        params.mobilityProportionHigh = 0.7f; //0.8f;
        params.mobilityProportionLow = 0.4f; //0.6f;

        params.responsivenessProportionHigh = 4.0f;
        params.responsivenessProportionLow = 0.4f;

        params.stallSpeed = DEFAULT_STALL_SPEED;
        params.recoverySpeed = DEFAULT_RECOVERY_SPEED;
        params.mobilityLooseStartSpeed = DEFAULT_MOBILITY_LOOSE_START_SPEED;
        
        params.pitchPower = 20;
        params.rollPower = 60;
        params.yawPower = 10;

        params.maxPitchSpeed = 0.6f;
        params.maxRollSpeed = 1.7f;
        params.maxYawSpeed = 0.15f;

        response = 200;
        params.pitchAcceleration = response * 2.0f;
        params.rollAcceleration = response * 4.0f;
        params.yawAcceleration = response;

        params.modelPath = "assets/F15Activeb.obj";//"assets/sf15b.obj";
        params.skipMaterialIndex = 6;//4;//7;

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

    params.pitchPower *= params.angularDamping.x;
    params.rollPower *= params.angularDamping.z;
    params.yawPower *= params.angularDamping.y;
    
    params.position = {0,400,-700};

    return params;
}
