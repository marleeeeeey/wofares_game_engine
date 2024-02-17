#pragma once
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/objects_factory.h>

class MapLoaderSystem
{
    entt::registry& registry;
    SDL_Renderer* renderer;
    GameState& gameState;
    ObjectsFactory objectsFactory;
    CoordinatesTransformer coordinatesTransformer;
    int tileWidth;
    int tileHeight;
    int colAndRowNumber;
    int miniWidth;
    int miniHeight;
    size_t createdTiles = 0;
    size_t invisibleTilesNumber = 0;
    std::shared_ptr<SDLTextureRAII> tilesetTexture;
public:
    MapLoaderSystem(entt::registry& registry, SDL_Renderer* renderer);
    void LoadMap(const std::string& filename);
    void UnloadMap();
private:
    void ParseTileLayer(const nlohmann::json& layer);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow);
};
