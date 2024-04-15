#pragma once
#include <glm/glm.hpp>
#include <utils/collision_flags.h>

struct Box2dBodyOptions
{
    struct Fixture
    {
        float density = 10.0f; // Density to calculate mass.
        float friction = 0.3f; // Friction to apply to the body.
        float restitution = 0.2f; // Restitution to apply to the body.
    } fixture;

    enum class Shape
    {
        Box,
        Capsule,
        Circle,
    } shape = Shape::Box;

    enum class Sensor
    {
        NoSensor, // No sensor.
        ThinSensorBelow, // Thin sensor below the body. Wight slightly less than the body.
    } sensor = Sensor::NoSensor;

    enum class MovementPolicy
    {
        Manual, // The object does not move.
        Box2dPhysics, // The object moves according to the Box2D physics. Behavior may be changed by AnglePolicy.
        Box2dPhysicsNoGravity, // The object moves according to the Box2D physics without gravity.
    } dynamic = MovementPolicy::Manual;

    enum class AnglePolicy
    {
        Dynamic, // The object rotates according to the Box2D physics.
        Fixed, // The object does not rotate.
        VelocityDirection // The object has the direction of the velocity vector.
    } anglePolicy = AnglePolicy::Dynamic;

    struct CollisionPolicy
    {
        CollisionFlags ownCategoryOfCollision = CollisionFlags::Default; // Collision flags to recieve.
        CollisionFlags collideWith = CollisionFlags::Default | CollisionFlags::Bullet; // Collision flags to accept.
    } collisionPolicy;

    enum class BulletPolicy
    {
        NotBullet, // The object is not a bullet.
        Bullet, // The object is a bullet.
    } bulletPolicy = BulletPolicy::NotBullet;

    struct Hitbox
    {
        glm::vec2 sizeWorld; // Size of the hitbox in the world coordinates.
    } hitbox;

    enum class EnabledPolicy
    {
        Enable,
        Disable,
    } enabledPolicy = EnabledPolicy::Enable;
};
