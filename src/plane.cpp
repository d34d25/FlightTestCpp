#include "plane.h"
#include "simpleTransform.h"
#include <iostream>

const float DEFAULT_LINEAR_DAMPING = 1.0f;
const float DEFAULT_ANGULAR_DAMPING = 3.0f;

const float DEFAULT_IDLE_THRUST = 37500; //365 units/s aprox

const float DEFAULT_STALL_SPEED = 130.0f;
const float DEFAULT_RECOVERY_SPEED = 140.0f;
const float DEFAULT_MOBILITY_LOOSE_START_SPEED = 250.0f;

const float DEFAULT_STALL_TORQUE_SPEED = 30.0f;

const float DEFAULT_PROPORTION_LOW = 0.6f;
const float DEFAULT_PROPORTION_HIGH = 0.8f;

const float DEFAULT_RETURN_SPEED_HIGH = 5000.0f * 0.8f;
const float DEFAULT_RETURN_SPEED_LOW = 5000.0f * 0.5f;

const float DEFAULT_MAX_THRUST = 175000.0f; //1700 units/s aprox
