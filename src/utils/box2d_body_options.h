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
};
