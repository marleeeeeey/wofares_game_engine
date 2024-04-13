#include "map_loader_system.h"
#include <SDL_image.h>
#include <box2d/b2_math.h>
#include <ecs/components/physics_components.h>
#include <fstream>
#include <my_cpp_utils/config.h>
#include <my_cpp_utils/math_utils.h>
#include <utils/entt_registry_wrapper.h>
#include <utils/factories/box2d_body_creator.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/logger.h>
#include <utils/math_utils.h>
#include <utils/sdl_texture_process.h>


MapLoaderSystem::MapLoaderSystem(EnttRegistryWrapper& registryWrapper, ResourceManager& resourceManager)
  : registryWrapper(registryWrapper), registry(registryWrapper.GetRegistry()), resourceManager(resourceManager),
    gameState(registry.get<GameOptions>(registry.view<GameOptions>().front())),
    objectsFactory(registryWrapper, resourceManager), coordinatesTransformer(registry)
{}

void MapLoaderSystem::LoadMap(const LevelInfo& levelInfo)
{
    currentLevelInfo = levelInfo;

    // Save map file path and load it as json.
    std::ifstream file(levelInfo.tiledMapPath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");

    nlohmann::json mapJson;
    file >> mapJson;

    // Load tileset texture and surface. Surface is used to search for invisible tiles.
    std::filesystem::path tilesetPath = ReadPathToTileset(mapJson);
    tilesetTexture = resourceManager.GetTexture(tilesetPath);
    tilesetSurface = resourceManager.GetSurface(tilesetPath);

    // Load background texture.
    gameState.levelOptions.backgroundInfo.texture = resourceManager.GetTexture(levelInfo.backgroundPath);

    // Assume all tiles are of the same size.
    tileWidth = mapJson["tilewidth"];
    tileHeight = mapJson["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    colAndRowNumber = utils::GetConfig<size_t, "MapLoaderSystem.tileSplitFactor">();
    miniWidth = tileWidth / colAndRowNumber;
    miniHeight = tileHeight / colAndRowNumber;

    // Iterate over each tile layer.
    for (const auto& layer : mapJson["layers"])
    {
        if (layer["type"] == "tilelayer" && layer["name"] == "terrain")
        {
            ParseTileLayer(layer);
        }
        else if (layer["type"] == "objectgroup")
        {
            ParseObjectLayer(layer);
        }
    }

    CalculateLevelBoundsWithBufferZone();

    // Log warnings.
    if (invisibleTilesNumber > 0)
        MY_LOG(info, "There are {}/{} tiles with invisible pixels", invisibleTilesNumber, createdTiles);
    if (createdTiles == 0)
    {
        MY_LOG(warn, "No tiles were created during map loading {}", levelInfo.tiledMapPath.string());
        if (invisibleTilesNumber > 0)
            MY_LOG(warn, "All tiles are invisible");
    }
}

void MapLoaderSystem::UnloadMap()
{
    auto& gameState = registry.get<GameOptions>(registry.view<GameOptions>().front());
    gameState.levelOptions.levelBox2dBounds = {};

    // Remove all entities that have a RenderingInfo component.
    for (auto entity : registry.view<RenderingComponent>())
        registryWrapper.Destroy(entity);

    // Remove all entities that have a PhysicalBody component.
    for (auto entity : registry.view<PhysicsComponent>())
        registryWrapper.Destroy(entity);

    if (Box2dObjectRAII::GetBodyCounter() != 0)
        MY_LOG(warn, "There are still {} Box2D bodies in the memory", Box2dObjectRAII::GetBodyCounter());
    else
        MY_LOG(debug, "All Box2D bodies were destroyed");
}

void MapLoaderSystem::ParseTileLayer(const nlohmann::json& layer)
{
    auto physicsWorld = gameState.physicsWorld;

    int layerCols = layer["width"];
    int layerRows = layer["height"];
    const auto& tiles = layer["data"];

    // Create entities for each tile.
    for (int layerRow = 0; layerRow < layerRows; ++layerRow)
    {
        for (int layerCol = 0; layerCol < layerCols; ++layerCol)
        {
            int tileId = tiles[layerCol + layerRow * layerCols];

            // Skip empty tiles.
            if (tileId <= 0)
                continue;

            ParseTile(tileId, layerCol, layerRow);
        }
    }
}

void MapLoaderSystem::ParseObjectLayer(const nlohmann::json& layer)
{
    for (const auto& object : layer["objects"])
    {
        if (object["type"] == "Player")
        {
            std::string objectName = object["name"];
            auto posWorld = glm::vec2(object["x"], object["y"]);
            objectsFactory.SpawnPlayer(posWorld, objectName);
        }

        if (object["type"] == "Portal")
        {
            std::string objectName = object["name"];
            auto posWorld = glm::vec2(object["x"], object["y"]);
            objectsFactory.SpawnPortal(posWorld, objectName);
        }
    }
}

void MapLoaderSystem::CalculateLevelBoundsWithBufferZone()
{
    auto& lb = gameState.levelOptions.levelBox2dBounds;
    auto& bz = gameState.levelOptions.bufferZone;
    MY_LOG(debug, "Level bounds: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
    lb.min -= bz;
    lb.max += bz;
    MY_LOG(
        debug, "Level bounds with buffer zone: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
}

void MapLoaderSystem::ParseTile(int tileId, int layerCol, int layerRow)
{
    auto physicsWorld = gameState.physicsWorld;

    SDL_Rect textureSrcRect = CalculateSrcRect(tileId, tileWidth, tileHeight, tilesetTexture);

    Box2dBodyCreator box2dBodyCreator(registry);

    // Create entities for each mini tile inside the tile.
    for (int miniRow = 0; miniRow < colAndRowNumber; ++miniRow)
    {
        for (int miniCol = 0; miniCol < colAndRowNumber; ++miniCol)
        {
            SDL_Rect miniTextureSrcRect{
                textureSrcRect.x + miniCol * miniWidth, textureSrcRect.y + miniRow * miniHeight, miniWidth, miniHeight};

            // Skip invisible tiles.
            {
                if (!tilesetSurface)
                    throw std::runtime_error("tilesetSurface is nullptr");

                if (IsTileInvisible(tilesetSurface->get(), miniTextureSrcRect))
                {
                    invisibleTilesNumber++;
                    continue;
                }
            }

            // Create tile entity.
            float miniTileWorldPositionX = layerCol * tileWidth + miniCol * miniWidth;
            float miniTileWorldPositionY = layerRow * tileHeight + miniRow * miniHeight;
            glm::vec2 miniTileWorldPosition{miniTileWorldPositionX, miniTileWorldPositionY};
            auto textureRect = TextureRect{tilesetTexture, miniTextureSrcRect};
            auto tileEntity = objectsFactory.SpawnTile(miniTileWorldPosition, miniWidth, textureRect);

            // Update level bounds.
            auto bodyRAII = registry.get<PhysicsComponent>(tileEntity).bodyRAII;
            const b2Vec2& bodyPosition = bodyRAII->GetBody()->GetPosition();
            auto& levelBounds = gameState.levelOptions.levelBox2dBounds;
            levelBounds.min = utils::Vec2Min(levelBounds.min, bodyPosition);
            levelBounds.max = utils::Vec2Max(levelBounds.max, bodyPosition);

            createdTiles++;
        }
    }
}

std::filesystem::path MapLoaderSystem::ReadPathToTileset(const nlohmann::json& mapJson)
{
    std::filesystem::path tilesetPath;

    if (mapJson.contains("tilesets") && mapJson["tilesets"][0].contains("source"))
    {
        // if map contains path to tileset.json
        //  "tilesets":[
        //         {
        //          "firstgid":1,
        //          "source":"tileset.json"
        //         }]
        std::filesystem::path tilesetJsonPath =
            currentLevelInfo.tiledMapPath.parent_path() / mapJson["tilesets"][0]["source"].get<std::string>();
        std::ifstream tilesetFile(tilesetJsonPath);
        if (!tilesetFile.is_open())
            throw std::runtime_error("Failed to open tileset file");

        nlohmann::json tilesetJson;
        tilesetFile >> tilesetJson;
        tilesetPath = currentLevelInfo.tiledMapPath.parent_path() / tilesetJson["image"].get<std::string>();
    }
    else if (mapJson.contains("tilesets") && mapJson["tilesets"][0].contains("image"))
    {
        // if map contains
        //  "tilesets":[
        //         {
        //          ...
        //          "firstgid":1,
        //          "image":"tileset.png",
        //          ...
        //         }]
        tilesetPath = currentLevelInfo.tiledMapPath.parent_path() / mapJson["tilesets"][0]["image"].get<std::string>();
    }
    else
    {
        throw std::runtime_error("[ReadPathToTileset] Failed to read path to tileset");
    }

    return tilesetPath;
}