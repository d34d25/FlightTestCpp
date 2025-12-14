#pragma once
#include<string>
#include "body.h"
#include "../headers/simpleTransform.h"

extern const float DEFAULT_STALL_SPEED;
extern const float DEFAULT_RECOVERY_SPEED;
extern const float DEFAULT_MOBILITY_LOOSE_START_SPEED;

extern const float MAX_THRUST;

using namespace std;

struct PlaneParams
{
    float scale;

    float hitboxWidth;
    float hitboxHeight;
    float hitboxLength;

    Vector3 angularDamping;
    Vector3 position;

    float lateralDragMultiplier; //more factor less drifting

    float maxThrust;
    float idleThrust;

    float acceleration;
    float brake;

    float returnSpeedHigh;
    float returnSpeedLow;

    //how much mobility it looses at low and high speeds
    float mobilityProportionLow;
    float mobilityProportionHigh;

    float responsivenessProportionLow;
    float responsivenessProportionHigh;

    float stallSpeed;
    float recoverySpeed;
    float mobilityLooseStartSpeed;

    float maxPitchSpeed;
    float maxRollSpeed;
    float maxYawSpeed;

    float pitchResponsiveness;
    float rollResponsiveness;
    float yawResponsiveness;

    //model variables
    const char* modelPath;
    int skipMaterialIndex;
};

enum class Planes
{
    DEBUG_PLANE
};

PlaneParams GetPlaneParams(Planes plane);