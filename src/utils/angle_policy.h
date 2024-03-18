#pragma once

enum class AnglePolicy
{
    Dynamic, // The object rotates according to the Box2D physics.
    Fixed, // The object does not rotate.
    VelocityDirection // The object has the direction of the velocity vector.
};
