#include "sdl_utils.h"

namespace utils
{

glm::vec2 GetCenterOfRect(const SDL_Rect& rect)
{
    return {rect.x + rect.w / 2, rect.y + rect.h / 2};
}

} // namespace utils