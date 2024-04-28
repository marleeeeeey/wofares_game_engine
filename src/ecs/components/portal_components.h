#pragma once
#include <box2d/box2d.h>
#include <optional>

struct PortalComponent
{
    float speed = 10.0f;
    float speedRestriction = 30.0f;
    float magnetForce = 150.0f;
    bool isSleeping = false;
    size_t foodCounter = 0;

    enum class PortalTargetType
    {
        Player,
        DestructibleParticle,
    };
    std::optional<std::pair<PortalTargetType, b2Vec2>> target;
};