#include "aux_systems.h"
#include <ecs/components/all_components.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

void ScatterSystem(entt::registry& registry, const glm::vec2& windowSize)
{
    auto view = registry.view<Position>();
    for (auto entity : view)
    {
        auto& pos = view.get<Position>(entity);

        pos.value.x = glm::linearRand(0.0f, windowSize.x);
        pos.value.y = glm::linearRand(0.0f, windowSize.y);
    }
}
