#pragma once
#include <ecs/components/all_components.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

class MapLoader
{
public:
    MapLoader(const std::string& filename, entt::registry& registry, SDL_Renderer* renderer);
private:
    void ParseTileLayer(const nlohmann::json& layer);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow);
private:
    entt::registry& registry;
    GameState& gameState;
    int tileWidth;
    int tileHeight;
    int colAndRowNumber;
    int miniWidth;
    int miniHeight;
    size_t createdTiles = 0;
    size_t invisibleTilesNumber = 0;
    std::shared_ptr<SDLTextureRAII> tilesetTexture;
};
