#include "body.h"
#include <iostream>

const float DEFAULT_ANGULAR_DAMPING = 3.0f;

const float FIXED_LINEAR_DAMPING = 5.0f;

Body3D::Body3D(float linearDamping, Vector3 angularDamping)
{
    force.x = 0.0f, force.y = 0.0f, force.z = 0.0f;

    torque.x = 0.0f, torque.y = 0.0f, torque.z = 0.0f;

    this->linearDamping = linearDamping;

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

    stallAngularTorque = 0.0f;
    stallAngularSpeed = 0.0f;

    linearAcceleration.x = 0.0f, linearAcceleration.y = 0.0f,
    linearAcceleration.z = 0.0f;

    angularAcceleration.x = 0.0f, angularAcceleration.y = 0.0f,
    angularAcceleration.z = 0.0f;

    mass = density * width * height * length;

    inertia.x = static_cast<float>((1.0f/12.0f) * mass * (height * height + length * length));
    inertia.y = static_cast<float>((1.0f/12.0f) * mass * (length * length + width * width));
    inertia.z = static_cast<float>((1.0f/12.0f) * mass * (width * width + height * height));
}


void Body3D::ApplyWorldTorque(Vector3 axis, float dt)
{
    float aAcc = 0.0f;

    if (inertia.x != 0.0f)
    {
        aAcc = stallAngularTorque / inertia.x;
    }

    stallAngularSpeed += aAcc * dt;

    if (stallAngularSpeed > 0.0001f)
    {
        float angle = stallAngularSpeed * dt;

        Quaternion deltaQ = QuaternionFromAxisAngle(axis, angle);

        transform.rotation = QuaternionMultiply(deltaQ, transform.rotation);

        transform.rotation = QuaternionNormalize(transform.rotation);
    }

    stallAngularSpeed *= Clamp(1 - DEFAULT_ANGULAR_DAMPING * dt, 0, 1);

    stallAngularTorque = 0.0f;
}

void Body3D::UpdateBody(float dt, int iterations)
{
    if(isStatic) return;

    dt /= iterations;

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


    linearVelocity.x *= Clamp(1.0f - linearDamping * dt, 0, 1);
    linearVelocity.y *= Clamp(1.0f - linearDamping * dt, 0, 1);
    linearVelocity.z *= Clamp(1.0f - linearDamping * dt, 0, 1);
    
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