#pragma once
#include "utils/resource_cashe.h"
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/objects_factory.h>

class MapLoaderSystem
{
    entt::registry& registry;
    ResourceCashe& resourceCashe;
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
    std::filesystem::path mapFilepath;
public:
    MapLoaderSystem(entt::registry& registry, ResourceCashe& resourceCashe);
    void LoadMap(const std::filesystem::path& filename);
    void UnloadMap();
private:
    void ParseTileLayer(const nlohmann::json& layer);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow);
};
