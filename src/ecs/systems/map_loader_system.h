#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <utils/RAII/sdl_RAII.h>
#include <utils/coordinates_transformer.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/objects_factory.h>
#include <utils/level_info.h>
#include <utils/resources/resource_manager.h>

class MapLoaderSystem
{
    EnttRegistryWrapper& registryWrapper;
    entt::registry& registry;
    ResourceManager& resourceManager;
    GameOptions& gameState;
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
    MapLoaderSystem(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager);
    void LoadMap(const LevelInfo& levelInfo);
    void UnloadMap();
private:
    void ParseTileLayer(const nlohmann::json& layer, ObjectsFactory::SpawnTileOption tileOptions);
    void ParseObjectLayer(const nlohmann::json& layer);
    void CalculateLevelBoundsWithBufferZone();
    void ParseTile(int tileId, int layerCol, int layerRow, ObjectsFactory::SpawnTileOption tileOptions);
private: // Low level functions.
    std::filesystem::path ReadPathToTileset(const nlohmann::json& mapJson);
};
