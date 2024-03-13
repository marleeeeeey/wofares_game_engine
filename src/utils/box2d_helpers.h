#include <box2d/box2d.h>

namespace utils
{
void DisableCollisionForTheBody(b2Body* body);

float CaclDistance(const b2Vec2& a, const b2Vec2& b);

} // namespace utils
