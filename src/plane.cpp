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

const float DEFAULT_PROPORTION_LOW = 0.6f;
const float DEFAULT_PROPORTION_HIGH = 0.8f;

const float DEFAULT_RETURN_SPEED_HIGH = 10000.0f * 0.8f;
const float DEFAULT_RETURN_SPEED_LOW = 10000.0f * 0.5f;
