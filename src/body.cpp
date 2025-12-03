#include "body.h"
#include <iostream>

const float FORWARD_DRAG_MULTIPLIER = 4.0f; //3.0f

const float ALT_ANGULAR_DAMPING = 3.0f;

Body3D::Body3D(float sideDrag, Vector3 angularDamping)
{
    force.x = 0.0f, force.y = 0.0f, force.z = 0.0f;
    
    alternateForce = {0.0f,0.0f,0.0f};

    alternateForceSpeed = {0.0f,0.0f,0.0f};

    torque.x = 0.0f, torque.y = 0.0f, torque.z = 0.0f;

    alternateTorque = {0,0,0};
    alternateAngularSpeed = {0,0,0};

    drag = 100.0f; //constant, same as mass

    this->lateralDragMultiplier = sideDrag;

    this->angularDamping.x = angularDamping.x;
    this->angularDamping.y = angularDamping.y;
    this->angularDamping.z = angularDamping.z;

    transform = Transform();

    transform.rotation = QuaternionIdentity();

    transform.scale.x = 1.0f, transform.scale.y = 1.0f,
    transform.scale.z = 1.0f;

    transform.translation.x = 0.0f, transform.translation.y = 0.0f,
    transform.translation.z = 0.0f;

    //private

    density = 100;

    height = 1, width = 1, length = 1;

    linearVelocity.x = 0.0f, linearVelocity.y = 0.0f,
    linearVelocity.z = 0.0f;

    angularVelocity.x = 0.0f, angularVelocity.y = 0.0f, 
    angularVelocity.z = 0.0f;

    worldAngularTorque = 0.0f;
    worldAngularSpeed = 0.0f;

    linearAcceleration.x = 0.0f, linearAcceleration.y = 0.0f,
    linearAcceleration.z = 0.0f;

    angularAcceleration.x = 0.0f, angularAcceleration.y = 0.0f,
    angularAcceleration.z = 0.0f;

    mass = density * width * height * length;

    inertia.x = static_cast<float>((1.0f/12.0f) * mass * (height * height + length * length));
    inertia.y = static_cast<float>((1.0f/12.0f) * mass * (length * length + width * width));
    inertia.z = static_cast<float>((1.0f/12.0f) * mass * (width * width + height * height));
}

//fix framerate / iterations dependency for damping

void Body3D::ApplyAlternateForce(float dt)
{
    Vector3 sAcc = {0,0,0};

    if(mass != 0.0f)
    {
        sAcc.x = alternateForce.x / mass;
        sAcc.y = alternateForce.y / mass;
        sAcc.z = alternateForce.z / mass;
    }

    alternateForceSpeed.x += sAcc.x * dt;
    alternateForceSpeed.y += sAcc.y * dt;
    alternateForceSpeed.z += sAcc.z * dt;

    transform.translation.x += alternateForceSpeed.x * dt;
    transform.translation.y += alternateForceSpeed.y * dt;
    transform.translation.z += alternateForceSpeed.z * dt;

    alternateForceSpeed.x *= Clamp(1 - FORWARD_DRAG_MULTIPLIER * dt, 0, 1);
    alternateForceSpeed.y *= Clamp(1 - FORWARD_DRAG_MULTIPLIER * dt, 0, 1);
    alternateForceSpeed.z *= Clamp(1 - FORWARD_DRAG_MULTIPLIER * dt, 0, 1);

    alternateForce.x = 0.0f;
    alternateForce.y = 0.0f;
    alternateForce.z = 0.0f;
}

void Body3D::ApplyAlernateTorque(float dt)
{
    Vector3 altAcc = {0,0,0};

    altAcc.x = 0.0f;
    altAcc.y = 0.0f;
    altAcc.z = 0.0f;

    if (!HasZeroInertia()) 
    {
        altAcc.x = alternateTorque.x / inertia.x;
        altAcc.y = alternateTorque.y / inertia.y;
        altAcc.z = alternateTorque.z / inertia.z;
    }

    alternateAngularSpeed.x += altAcc.x * dt;
    alternateAngularSpeed.y += altAcc.y * dt;
    alternateAngularSpeed.z += altAcc.z * dt;

    float angSpeed = Vector3Length(alternateAngularSpeed);

    if(angSpeed > 0.0f)
    {
        Vector3 axis;

        axis.x = alternateAngularSpeed.x / angSpeed;
        axis.y = alternateAngularSpeed.y / angSpeed;
        axis.z = alternateAngularSpeed.z / angSpeed;

        float angle = angSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis,angle);

        transform.rotation = QuaternionMultiply(transform.rotation, deltaQ);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    alternateAngularSpeed.x *= Clamp(1.0f - ALT_ANGULAR_DAMPING * dt, 0, 1);
    alternateAngularSpeed.y *= Clamp(1.0f - ALT_ANGULAR_DAMPING * dt, 0, 1);
    alternateAngularSpeed.z *= Clamp(1.0f - ALT_ANGULAR_DAMPING * dt, 0, 1);

    alternateTorque.x = 0.0f;
    alternateTorque.y = 0.0f;
    alternateTorque.z = 0.0f;
}

void Body3D::ApplyAlternateWorldTorque(Vector3 axis, float dt)
{
    float aAcc = 0.0f;

    if (inertia.x != 0.0f)
    {
        aAcc = worldAngularTorque / inertia.x;
    }

    worldAngularSpeed += aAcc * dt;

    if (worldAngularSpeed > 0.0001f)
    {
        float angle = worldAngularSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis, angle);

        transform.rotation = QuaternionMultiply(deltaQ, transform.rotation);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    worldAngularSpeed *= Clamp(1 - ALT_ANGULAR_DAMPING * dt, 0, 1);

    worldAngularTorque = 0.0f;
}

void Body3D::UpdateBody(float dt, int iterations)
{   
    dt /= iterations;

    Vector3 forward = GetLocalForwardVector(transform);
    forward = Vector3Normalize(forward);
    float forwardSpeed = Vector3DotProduct(linearVelocity, forward);

    Vector3 forwardVel = Vector3Scale(forward, forwardSpeed);
    Vector3 lateralVel = Vector3Subtract(linearVelocity, forwardVel);

    //forward drag

    Vector3 fowardVelVector = Vector3Subtract(linearVelocity, lateralVel);

    float forwardDrag = drag * FORWARD_DRAG_MULTIPLIER;

    float maxForwardDragFactor = GetMass() / dt;

    if(forwardDrag > maxForwardDragFactor)
    {
        forwardDrag = maxForwardDragFactor;
    }

    Vector3 dragForce = Vector3Scale(fowardVelVector, -forwardDrag);
    force = Vector3Add(force, dragForce);

    //lateral drag

    float lateralDrag = drag * lateralDragMultiplier;

    float maxLateralDragFactor = GetMass() / dt;

    if(lateralDrag > maxLateralDragFactor) 
    {
        lateralDrag = maxLateralDragFactor;
    }

    Vector3 lateralForce = Vector3Scale(lateralVel, -lateralDrag);
    force = Vector3Add(force, lateralForce);

    //linearl update (world space)

    linearAcceleration.x = 0.0f;
    linearAcceleration.y = 0.0f;
    linearAcceleration.z = 0.0f;

    if(!HasZeroMass())
    {
        linearAcceleration.x = force.x / mass;
        linearAcceleration.y = force.y / mass;
        linearAcceleration.z = force.z / mass;
    }

    linearVelocity.x += linearAcceleration.x * dt;
    linearVelocity.y += linearAcceleration.y * dt;
    linearVelocity.z += linearAcceleration.z * dt;

    transform.translation.x += linearVelocity.x * dt;
    transform.translation.y += linearVelocity.y * dt;
    transform.translation.z += linearVelocity.z * dt;
    
    force.x = 0.0f;
    force.y = 0.0f;
    force.z = 0.0f;

    //angular update (local space)

    angularAcceleration.x = 0.0f;
    angularAcceleration.y = 0.0f;
    angularAcceleration.z = 0.0f;

    if (!HasZeroInertia()) 
    {
        angularAcceleration.x = torque.x / inertia.x;
        angularAcceleration.y = torque.y / inertia.y;
        angularAcceleration.z = torque.z / inertia.z;
    }

    angularVelocity.x += angularAcceleration.x * dt;
    angularVelocity.y += angularAcceleration.y * dt;
    angularVelocity.z += angularAcceleration.z * dt;

    float angSpeed = Vector3Length(angularVelocity);

    if(angSpeed > 0.0f)
    {
        Vector3 axis;

        axis.x = angularVelocity.x / angSpeed;
        axis.y = angularVelocity.y / angSpeed;
        axis.z = angularVelocity.z / angSpeed;

        float angle = angSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis,angle);

        transform.rotation = QuaternionMultiply(transform.rotation, deltaQ);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    angularVelocity.x *= Clamp(1.0f - angularDamping.x * dt, 0, 1);
    angularVelocity.y *= Clamp(1.0f - angularDamping.y * dt, 0, 1);
    angularVelocity.z *= Clamp(1.0f - angularDamping.z * dt, 0, 1);

    torque.x = 0.0f;
    torque.y = 0.0f;
    torque.z = 0.0f;
}

