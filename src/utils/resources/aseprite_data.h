#pragma once
#include "SDL_rect.h"
#include <nlohmann/json.hpp>
#include <unordered_map>

/**
 * Example of JSON data from Aseprite:
 * {
 *   "frames": {
 *     "m_walk 0.png": {
 *       "frame": { "x": 0, "y": 0, "w": 9, "h": 19 },
 *       ...
 *       "duration": 100
 *     },
 *     ...
 *   },
 *   "meta": {
 *     ...
 *     "image": "SPRITE-SHEET.png",
 *     "frameTags": [
 *       { "name": "Idle", "from": 0, "to": 3, ... },
 *       { "name": "Run", "from": 4, "to": 9, ... },
 *       { "name": "Hitbox", "from": 10, "to": 10, ... }
 *     ],
 *     ...
 *   }
 * }
 */
struct AsepriteData
{
    struct Frame
    {
        size_t originalIndex;
        float duration_seconds;
        SDL_Rect rectInTexture;
    };

    struct FrameTag
    {
        std::string name;
        size_t from;
        size_t to;
    };

    std::unordered_map<std::string, FrameTag> frameTags;
    std::vector<Frame> frames;
    std::string texturePath;
};

AsepriteData LoadAsepriteData(const nlohmann::json& asepriteJsonData);
