#pragma once
#include<string>
#include "body.h"
#include "../headers/simpleTransform.h"

extern const float DEFAULT_LINEAR_DAMPING;
extern const float DEFAULT_ANGULAR_DAMPING;

extern const float DEFAULT_IDLE_THRUST;

extern const float DEFAULT_STALL_SPEED;
extern const float DEFAULT_RECOVERY_SPEED;
extern const float DEFAULT_MOBILITY_LOOSE_START_SPEED;

extern const float DEFAULT_STALL_TORQUE_SPEED;
extern const float DEFAULT_STALL_TORQUE_STOP_SPEED;

extern const float DEFAULT_MAX_STALL_TORQUE;

extern const float DEFAULT_PROPORTION_LOW;
extern const float DEFAULT_PROPORTION_HIGH;

extern const float DEFAULT_RETURN_SPEED_HIGH;
extern const float DEFAULT_RETURN_SPEED_LOW;

using namespace std;

struct PlaneParams
{
    float hitboxWidth = 11;
    float hitboxHeight = 1;
    float hitboxLength = 20;

    float linearDamping = DEFAULT_LINEAR_DAMPING;

    Vector3 angularDamping;
    Vector3 position;

    float maxThrust = 350000.0f;
    float idleThrust = DEFAULT_IDLE_THRUST;

    float returnSpeedHigh = DEFAULT_RETURN_SPEED_HIGH;
    float returnSpeedLow = DEFAULT_RETURN_SPEED_LOW;

    float stallSpeed = DEFAULT_STALL_SPEED;
    float recoverySpeed = DEFAULT_RECOVERY_SPEED;

    float mobilityLooseStartSpeed = DEFAULT_MOBILITY_LOOSE_START_SPEED;

    float stallTorqueSpeed = DEFAULT_STALL_TORQUE_SPEED;
    float stallTorqueStopSpeed = DEFAULT_STALL_TORQUE_STOP_SPEED;

    float maxStallTorque = DEFAULT_MAX_STALL_TORQUE;

    //how much mobility it looses at low and high speeds
    float proportionLow = DEFAULT_PROPORTION_LOW;
    float proportionHigh = DEFAULT_PROPORTION_HIGH;

    float acceleration = 21000.0f;
    float brake = 18000.0f;

    float pitchPower = 70.0f;
    float rollPower = 190.0f;
    float yawPower = 25.0f;

    float maxPitchSpeed = 1.0f;
    float maxRollSpeed = 1.8f;
    float maxYawSpeed = 0.23f;

    const char* modelPath = "";
};


class Plane
{
private:

    bool stalling = false;
    float angleSpeed = 0.0f;
    float stallTorque = 0.0f;

    float originalMaxPitchSpeed;
    float originalMaxRollSpeed;
    float originalMaxYawSpeed;

public:

    bool returnToIdle = true;
    bool hasInput = false;

    float thrust = 0.0f;

    PlaneParams params;

    Body3D body;

    Plane() = default;

    Plane(PlaneParams params);

    void UpdatePlane(float dt, int iterations);

    //getters

    inline float GetMaxSpeed()
    {
        float maxThrust = params.idleThrust + (params.maxThrust - params.idleThrust);

        float maxVelocity = maxThrust  / body.GetMass();

        float correctionFactor = 0.97f;

        return (maxVelocity / body.linearDamping) * correctionFactor;
    }

    inline float GetIdleSpeed()
    {
        float maxVelocityIdle = params.idleThrust / body.GetMass();

        float correctionFactor = 0.97f;

        return(maxVelocityIdle / body.linearDamping) * correctionFactor;
    }

    inline float GetSpeed()
    {
        return Vector3Length(body.linearVelocity);
    }
};