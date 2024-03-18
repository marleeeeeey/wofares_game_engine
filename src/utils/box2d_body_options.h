#pragma once

struct Box2dBodyOptions
{
    enum class Shape
    {
        Box,
        Capsule,
        Circle,
    } shape = Shape::Box;

    struct Fixture
    {
        float density = 10.0f; // Density to calculate mass.
        float friction = 0.3f; // Friction to apply to the body.
        float restitution = 0.2f; // Restitution to apply to the body.
    } fixture;

    enum class Sensor
    {
        NoSensor,
        ThinSensorBelow,
    } sensor = Sensor::NoSensor;

    enum class DynamicOptions
    {
        Static,
        Dynamic,
    } dynamic = DynamicOptions::Static;

    enum class AnglePolicy
    {
        Dynamic, // The object rotates according to the Box2D physics.
        Fixed, // The object does not rotate.
        VelocityDirection // The object has the direction of the velocity vector.
    } anglePolicy = AnglePolicy::Dynamic;
};
