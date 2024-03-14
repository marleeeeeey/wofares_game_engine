#include "box2d_helpers.h"

namespace utils
{
void DisableCollisionForTheBody(b2Body* body)
{
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture != nullptr)
    {
        b2Filter filter = fixture->GetFilterData();
        filter.maskBits = 0x0000; // Mask to ignore all collisions.
        fixture->SetFilterData(filter);
        fixture = fixture->GetNext();
    }
};

} // namespace utils