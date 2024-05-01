#include "box2d_math.h"
#include <limits>
#include <utils/vec_operators.h>

b2Vec2 TranformBox2dPolygonShapeToRectagleSize(const b2PolygonShape* polygonShape)
{
    b2Vec2 topLeft = b2Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    b2Vec2 bottomRight = b2Vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    for (int i = 0; i < polygonShape->m_count; i++)
    {
        const b2Vec2 vertexWorld = polygonShape->m_vertices[i];
        topLeft = b2Min(topLeft, vertexWorld);
        bottomRight = b2Max(bottomRight, vertexWorld);
    }

    return bottomRight - topLeft;
}
