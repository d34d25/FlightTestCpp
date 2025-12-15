#include "body.h"
#include <iostream>

const float FORWARD_DRAG_MULTIPLIER = 4.0f; //3.0f

const float ALT_ANGULAR_DAMPING = 3.0f;

Body3D::Body3D(float sideDrag, Vector3 angularDamping)
{
    force.x = 0.0f, force.y = 0.0f, force.z = 0.0f;
    
    alternateForce = {0.0f,0.0f,0.0f};

    alternateLinearVelocity = {0.0f,0.0f,0.0f};

    torque.x = 0.0f, torque.y = 0.0f, torque.z = 0.0f;

    alternateTorque = {0,0,0};
    alternateAngularVelocity = {0,0,0};

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
    worldAngularVelocity = 0.0f;

    mass = density * width * height * length;

    inertia.x = static_cast<float>((1.0f/12.0f) * mass * (height * height + length * length));
    inertia.y = static_cast<float>((1.0f/12.0f) * mass * (length * length + width * width));
    inertia.z = static_cast<float>((1.0f/12.0f) * mass * (width * width + height * height));
}


void Body3D::AlternateUpdateBody(float dt)
{
    Vector3 forward = GetWorldForwardVector(transform);

    float altDrag = linearDrag * FORWARD_DRAG_MULTIPLIER;

    float maxAltDrag = GetMass() / dt;

    if (altDrag > maxAltDrag) altDrag = maxAltDrag;

    Vector3 altDragForce = Vector3Scale(alternateLinearVelocity, -altDrag);
    alternateForce = Vector3Add(alternateForce, altDragForce);

    Vector3 sAcc = {0,0,0};

    if(mass != 0.0f)
    {
        sAcc.x = alternateForce.x / mass;
        sAcc.y = alternateForce.y / mass;
        sAcc.z = alternateForce.z / mass;
    }

    alternateLinearVelocity.x += sAcc.x * dt;
    alternateLinearVelocity.y += sAcc.y * dt;
    alternateLinearVelocity.z += sAcc.z * dt;

    transform.translation.x += alternateLinearVelocity.x * dt;
    transform.translation.y += alternateLinearVelocity.y * dt;
    transform.translation.z += alternateLinearVelocity.z * dt;

    alternateForce.x = 0.0f;
    alternateForce.y = 0.0f;
    alternateForce.z = 0.0f;

    //alt torque

    float pitchDrag = angularDrag * ALT_ANGULAR_DAMPING;
    float rollDrag = angularDrag * ALT_ANGULAR_DAMPING;
    float yawDrag = angularDrag * ALT_ANGULAR_DAMPING;

    float maxPitchDragFactor = GetInertia().x / dt;
    float maxRollDragFactor = GetInertia().z / dt;
    float maxYawDragFactor = GetInertia().y / dt;

    if(pitchDrag > maxPitchDragFactor) pitchDrag = maxPitchDragFactor;
    if(rollDrag > maxRollDragFactor) rollDrag = maxRollDragFactor;
    if(yawDrag > maxYawDragFactor) yawDrag = maxYawDragFactor;

    float pitchT = alternateAngularVelocity.x * -pitchDrag;
    float rollT = alternateAngularVelocity.z * -rollDrag;
    float yawT = alternateAngularVelocity.y * -yawDrag;

    Vector3 dragTorque = {pitchT, yawT, rollT};

    alternateTorque = Vector3Add(alternateTorque, dragTorque);

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

    alternateAngularVelocity.x += altAcc.x * dt;
    alternateAngularVelocity.y += altAcc.y * dt;
    alternateAngularVelocity.z += altAcc.z * dt;

    float angSpeed = Vector3Length(alternateAngularVelocity);

    if(angSpeed > 0.0f)
    {
        Vector3 axis;

        axis.x = alternateAngularVelocity.x / angSpeed;
        axis.y = alternateAngularVelocity.y / angSpeed;
        axis.z = alternateAngularVelocity.z / angSpeed;

        float angle = angSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis,angle);

        transform.rotation = QuaternionMultiply(transform.rotation, deltaQ);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    alternateTorque.x = 0.0f;
    alternateTorque.y = 0.0f;
    alternateTorque.z = 0.0f;
}

void Body3D::ApplyAlternateWorldTorque(Vector3 axis, float dt)
{   
    float worldRotationDrag = angularDrag * ALT_ANGULAR_DAMPING;

    float maxWorldRotDragFractor = GetInertia().x / dt;

    if (worldRotationDrag > maxWorldRotDragFractor) worldRotationDrag = maxWorldRotDragFractor;

    float worldT = worldAngularVelocity * -worldRotationDrag;

    worldAngularTorque += worldT;

    float aAcc = 0.0f;

    if (inertia.x != 0.0f)
    {
        aAcc = worldAngularTorque / inertia.x;
    }

    worldAngularVelocity += aAcc * dt;

    if (worldAngularVelocity > 0.0001f)
    {
        float angle = worldAngularVelocity * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis, angle);

        transform.rotation = QuaternionMultiply(deltaQ, transform.rotation);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    worldAngularTorque = 0.0f;
}

void Body3D::UpdateBody(float dt)
{   
    Vector3 forward = GetWorldForwardVector(transform);

    float forwardSpeed = Vector3DotProduct(linearVelocity, forward);

    Vector3 forwardVel = Vector3Scale(forward, forwardSpeed);
    Vector3 lateralVel = Vector3Subtract(linearVelocity, forwardVel);

    //forward drag

    Vector3 fowardVelVector = Vector3Subtract(linearVelocity, lateralVel);

    float forwardDrag = linearDrag * FORWARD_DRAG_MULTIPLIER;

    float maxForwardDragFactor = GetMass() / dt;

    if(forwardDrag > maxForwardDragFactor)
    {
        forwardDrag = maxForwardDragFactor;
    }

    Vector3 dragForce = Vector3Scale(fowardVelVector, -forwardDrag);
    force = Vector3Add(force, dragForce);

    //lateral drag

    float lateralDrag = linearDrag * lateralDragMultiplier;

    float maxLateralDragFactor = GetMass() / dt;

    if(lateralDrag > maxLateralDragFactor) 
    {
        lateralDrag = maxLateralDragFactor;
    }

    Vector3 lateralForce = Vector3Scale(lateralVel, -lateralDrag);
    force = Vector3Add(force, lateralForce);

    //linear update (world space)

    Vector3 linearAcceleration = {0.0f,0.0f,0.0f};

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

    //angular drag

    float pitchDrag = angularDrag * angularDamping.x;
    float rollDrag = angularDrag * angularDamping.z;
    float yawDrag = angularDrag * angularDamping.y;

    float maxPitchDragFactor = GetInertia().x / dt;
    float maxRollDragFactor = GetInertia().z / dt;
    float maxYawDragFactor = GetInertia().y / dt;

    if(pitchDrag > maxPitchDragFactor) pitchDrag = maxPitchDragFactor;
    if(rollDrag > maxRollDragFactor) rollDrag = maxRollDragFactor;
    if(yawDrag > maxYawDragFactor) yawDrag = maxYawDragFactor;

    float pitchT = angularVelocity.x * -pitchDrag;
    float rollT = angularVelocity.z * -rollDrag;
    float yawT = angularVelocity.y * -yawDrag;

    Vector3 dragTorque = {pitchT, yawT, rollT};

    torque = Vector3Add(torque, dragTorque);

    //angular update (local space)
    Vector3 angularAcceleration = {0.0f,0.0f,0.0f};

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

    torque.x = 0.0f;
    torque.y = 0.0f;
    torque.z = 0.0f;
}

void Body3D::SingleBodyUpdate(float dt)
{
    Vector3 forward = GetWorldForwardVector(transform);

    //linear

    //alt
    float altDrag = linearDrag * FORWARD_DRAG_MULTIPLIER;

    float maxAltDrag = GetMass() / dt;

    if (altDrag > maxAltDrag) altDrag = maxAltDrag;

    Vector3 altDragForce = Vector3Scale(alternateLinearVelocity, -altDrag);
    alternateForce = Vector3Add(alternateForce, altDragForce);

    Vector3 alternateLinearAcc = {0,0,0};

    if(mass != 0.0f)
    {
        alternateLinearAcc.x = alternateForce.x / mass;
        alternateLinearAcc.y = alternateForce.y / mass;
        alternateLinearAcc.z = alternateForce.z / mass;
    }

    alternateLinearVelocity.x += alternateLinearAcc.x * dt;
    alternateLinearVelocity.y += alternateLinearAcc.y * dt;
    alternateLinearVelocity.z += alternateLinearAcc.z * dt;


    //normal

    float forwardSpeed = Vector3DotProduct(linearVelocity, forward);

    Vector3 forwardVel = Vector3Scale(forward, forwardSpeed);
    Vector3 lateralVel = Vector3Subtract(linearVelocity, forwardVel);

    //forward drag

    Vector3 fowardVelVector = Vector3Subtract(linearVelocity, lateralVel);

    float forwardDrag = linearDrag * FORWARD_DRAG_MULTIPLIER;

    float maxForwardDragFactor = GetMass() / dt;

    if(forwardDrag > maxForwardDragFactor)
    {
        forwardDrag = maxForwardDragFactor;
    }

    Vector3 dragForce = Vector3Scale(fowardVelVector, -forwardDrag);
    force = Vector3Add(force, dragForce);

    //lateral drag

    float lateralDrag = linearDrag * lateralDragMultiplier;

    float maxLateralDragFactor = GetMass() / dt;

    if(lateralDrag > maxLateralDragFactor) 
    {
        lateralDrag = maxLateralDragFactor;
    }

    Vector3 lateralForce = Vector3Scale(lateralVel, -lateralDrag);
    force = Vector3Add(force, lateralForce);

    //linear update (world space)

    Vector3 linearAcceleration = {0.0f,0.0f,0.0f};

    if(!HasZeroMass())
    {
        linearAcceleration.x = force.x / mass;
        linearAcceleration.y = force.y / mass;
        linearAcceleration.z = force.z / mass;
    }

    linearVelocity.x += linearAcceleration.x * dt;
    linearVelocity.y += linearAcceleration.y * dt;
    linearVelocity.z += linearAcceleration.z * dt;

    //linear integration

    Vector3 totalVelocity = Vector3Add(linearVelocity, alternateLinearVelocity);

    transform.translation.x += totalVelocity.x * dt;
    transform.translation.y += totalVelocity.y * dt;
    transform.translation.z += totalVelocity.z * dt;

    //alt
    alternateForce.x = 0.0f;
    alternateForce.y = 0.0f;
    alternateForce.z = 0.0f;

    //normal
    force.x = 0.0f;
    force.y = 0.0f;
    force.z = 0.0f;


    //angular

    //alt

    float altPitchDrag = angularDrag * ALT_ANGULAR_DAMPING;
    float altRollDrag = angularDrag * ALT_ANGULAR_DAMPING;
    float altYawDrag = angularDrag * ALT_ANGULAR_DAMPING;

    float altMaxPitchDragFactor = GetInertia().x / dt;
    float altMaxRollDragFactor = GetInertia().z / dt;
    float altMaxYawDragFactor = GetInertia().y / dt;

    if(altPitchDrag > altMaxPitchDragFactor) altPitchDrag = altMaxPitchDragFactor;
    if(altRollDrag > altMaxRollDragFactor) altRollDrag = altMaxRollDragFactor;
    if(altYawDrag > altMaxYawDragFactor) altYawDrag = altMaxYawDragFactor;

    float altPitchT = alternateAngularVelocity.x * -altPitchDrag;
    float altRollT = alternateAngularVelocity.z * -altRollDrag;
    float altYawT = alternateAngularVelocity.y * -altYawDrag;

    Vector3 altDragTorque = {altPitchT, altYawT, altRollT};

    alternateTorque = Vector3Add(alternateTorque, altDragTorque);

    Vector3 altAngularAcc = {0.0f,0.0f,0.0f};

    if (!HasZeroInertia()) 
    {
        altAngularAcc.x = alternateTorque.x / inertia.x;
        altAngularAcc.y = alternateTorque.y / inertia.y;
        altAngularAcc.z = alternateTorque.z / inertia.z;
    }

    alternateAngularVelocity.x += altAngularAcc.x * dt;
    alternateAngularVelocity.y += altAngularAcc.y * dt;
    alternateAngularVelocity.z += altAngularAcc.z * dt;

    //normal

    float pitchDrag = angularDrag * angularDamping.x;
    float rollDrag = angularDrag * angularDamping.z;
    float yawDrag = angularDrag * angularDamping.y;

    float maxPitchDragFactor = GetInertia().x / dt;
    float maxRollDragFactor = GetInertia().z / dt;
    float maxYawDragFactor = GetInertia().y / dt;

    if(pitchDrag > maxPitchDragFactor) pitchDrag = maxPitchDragFactor;
    if(rollDrag > maxRollDragFactor) rollDrag = maxRollDragFactor;
    if(yawDrag > maxYawDragFactor) yawDrag = maxYawDragFactor;

    float pitchT = angularVelocity.x * -pitchDrag;
    float rollT = angularVelocity.z * -rollDrag;
    float yawT = angularVelocity.y * -yawDrag;

    Vector3 dragTorque = {pitchT, yawT, rollT};

    torque = Vector3Add(torque, dragTorque);

    Vector3 angularAcceleration = {0.0f,0.0f,0.0f};

    if (!HasZeroInertia()) 
    {
        angularAcceleration.x = torque.x / inertia.x;
        angularAcceleration.y = torque.y / inertia.y;
        angularAcceleration.z = torque.z / inertia.z;
    }

    angularVelocity.x += angularAcceleration.x * dt;
    angularVelocity.y += angularAcceleration.y * dt;
    angularVelocity.z += angularAcceleration.z * dt;

    //angular integration

    Vector3 totalAngularVelocity = Vector3Add(alternateAngularVelocity, angularVelocity);

    float angSpeed = Vector3Length(totalAngularVelocity);

    if(angSpeed > 0.0f)
    {
        Vector3 axis;

        axis.x = totalAngularVelocity.x / angSpeed;
        axis.y = totalAngularVelocity.y / angSpeed;
        axis.z = totalAngularVelocity.z / angSpeed;

        float angle = angSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis,angle);

        transform.rotation = QuaternionMultiply(transform.rotation, deltaQ);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    //alt
    alternateTorque.x = 0.0f;
    alternateTorque.y = 0.0f;
    alternateTorque.z = 0.0f;
    //normal
    torque.x = 0.0f;
    torque.y = 0.0f;
    torque.z = 0.0f;
}











/*
GUIDE FOR DECIDING ROTATION ORDER:

LAST MEANS HIGHEST AUTHORITY
FIRST MEANS LOWEST AUTHORITY

The general rule of thumb for physical systems is:

World Rotations First: 
Apply rotations that are fixed relative to the world 
(e.g., a simple gravity-induced torque, or a powerful, absolute tumble like your stall). 
This sets the baseline global frame.

Local Rotations Second: 
Apply rotations that are defined relative to the body's internal frame 
(e.g., thrust-induced pitch, rudder yaw, local drag). 
These rotations use the baseline global frame as their reference.

UpdateBody and AlternateUpdateBody are in local space

while ApplyAlternateWorldTorque is in world space

*/