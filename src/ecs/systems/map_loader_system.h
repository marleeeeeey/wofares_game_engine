#pragma once
#include "utils/level_info.h"
#include "utils/resource_manager.h"
#include "utils/sdl_RAII.h"
#include <ecs/components/game_components.h>
#include <entt/entt.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/coordinates_transformer.h>
#include <utils/objects_factory.h>

class MapLoaderSystem
{
    entt::registry& registry;
    ResourceManager& resourceManager;
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
    std::shared_ptr<SDLSurfaceRAII> tilesetSurface; // Optional. Used when Streaming access is needed.
    LevelInfo currentLevelInfo;
public:
    MapLoaderSystem(entt::registry& registry, ResourceManager& resourceManager);
    void LoadMap(const LevelInfo& levelInfo);
    void UnloadMap();
private:
    void ParseTileLayer(const nlohmann::json& layer);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow);
private: // Low level functions.
    std::filesystem::path ReadPathToTileset(const nlohmann::json& mapJson);
};
