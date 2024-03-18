#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

namespace utils
{

glm::vec2 GetCenterOfRect(const SDL_Rect& rect);

} // namespace utils
