#include "plane.h"
#include "simpleTransform.h"
#include <iostream>

//fixed for ALL planes
const float FIXED_LINEAR_DAMPING = 5.0f;


const float DEFAULT_ANGULAR_DAMPING = 3.0f;
const float DEFAULT_IDLE_THRUST = 187500; //365 units/s aprox

const float DEFAULT_STALL_SPEED = 140.0f;
const float DEFAULT_RECOVERY_SPEED = 210.0f;
const float DEFAULT_MOBILITY_LOOSE_START_SPEED = 250.0f;

const float DEFAULT_STALL_TORQUE_SPEED = 45.0f;

const float DEFAULT_PROPORTION_LOW = 0.6f;
const float DEFAULT_PROPORTION_HIGH = 0.8f;

const float DEFAULT_RETURN_SPEED_HIGH = 20000;
const float DEFAULT_RETURN_SPEED_LOW = 12500;

const float DEFAULT_MAX_THRUST = 875000.0f; //1700 units/s aprox
