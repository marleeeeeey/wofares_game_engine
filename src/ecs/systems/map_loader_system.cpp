#include "map_loader_system.h"
#include "my_common_cpp_utils/Logger.h"
#include <SDL_image.h>
#include <box2d/b2_math.h>
#include <fstream>
#include <my_common_cpp_utils/MathUtils.h>
#include <utils/box2d_body_creator.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/texture_process.h>

MapLoaderSystem::MapLoaderSystem(entt::registry& registry, ResourceManager& resourceManager)
  : registry(registry), resourceManager(resourceManager),
    gameState(registry.get<GameState>(registry.view<GameState>().front())), objectsFactory(registry, resourceManager),
    coordinatesTransformer(registry)
{}

void MapLoaderSystem::LoadMap(const std::filesystem::path& filename)
{
    // Save map file path and load it as json.
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");
    mapFilepath = filename;
    nlohmann::json mapJson;
    file >> mapJson;

    // Load tileset texture and surface. Surface is used to search for invisible tiles.
    std::filesystem::path tilesetPath = ReadPathToTileset(mapJson);
    tilesetTexture = resourceManager.GetTexture(tilesetPath);
    if (gameState.levelOptions.preventCreationInvisibleTiles)
        tilesetSurface = resourceManager.GetSurface(tilesetPath);
    else
        tilesetSurface = nullptr;

    // Assume all tiles are of the same size.
    tileWidth = mapJson["tilewidth"];
    tileHeight = mapJson["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    colAndRowNumber = gameState.levelOptions.miniTileResolution;
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
        MY_LOG_FMT(warn, "There are {}/{} tiles with invisible pixels", invisibleTilesNumber, createdTiles);
    if (createdTiles == 0)
    {
        MY_LOG_FMT(warn, "No tiles were created during map loading {}", mapFilepath.string());
        if (invisibleTilesNumber > 0)
            MY_LOG(warn, "All tiles are invisible");
    }
};

void MapLoaderSystem::UnloadMap()
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    gameState.levelOptions.levelBox2dBounds = {};

    // Remove all entities that have a RenderingInfo component.
    for (auto entity : registry.view<RenderingInfo>())
        registry.destroy(entity);

    // Remove all entities that have a PhysicalBody component.
    for (auto entity : registry.view<PhysicsInfo>())
        registry.destroy(entity);

    if (Box2dObjectRAII::GetBodyCounter() != 0)
        MY_LOG_FMT(warn, "There are still {} Box2D bodies in the memory", Box2dObjectRAII::GetBodyCounter());
    else
        MY_LOG(debug, "All Box2D bodies were destroyed");
};

void MapLoaderSystem::ParseTileLayer(const nlohmann::json& layer)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

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
    // TODO: implement loading the background texture from main json file.
    // for (const auto& props : layer["properties"])
    // {
    //     if (props["name"] == "background")
    //     {
    //         auto backgroundTexturePath = mapFilepath.parent_path() / props["value"];
    //         gameState.levelOptions.backgroundInfo.texture = resourceManager.GetTexture(backgroundTexturePath);
    //     }
    // }

    for (const auto& object : layer["objects"])
    {
        if (object["type"] == "PlayerPosition")
        {
            auto playerSdlWorldPos = glm::vec2(object["x"], object["y"]);
            objectsFactory.createPlayer(playerSdlWorldPos);
        }
    }
}

void MapLoaderSystem::CalculateLevelBoundsWithBufferZone()
{
    auto& lb = gameState.levelOptions.levelBox2dBounds;
    auto& bz = gameState.levelOptions.bufferZone;
    MY_LOG_FMT(debug, "Level bounds: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
    lb.min -= bz;
    lb.max += bz;
    MY_LOG_FMT(
        debug, "Level bounds with buffer zone: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
}

void MapLoaderSystem::ParseTile(int tileId, int layerCol, int layerRow)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

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
            if (gameState.levelOptions.preventCreationInvisibleTiles)
            {
                if (!tilesetSurface)
                    throw std::runtime_error("tilesetSurface is nullptr");

                if (IsTileInvisible(tilesetSurface->get(), miniTextureSrcRect))
                {
                    invisibleTilesNumber++;
                    continue;
                }
            }

            float miniTileWorldPositionX = layerCol * tileWidth + miniCol * miniWidth;
            float miniTileWorldPositionY = layerRow * tileHeight + miniRow * miniHeight;
            glm::vec2 miniTileWorldPosition{miniTileWorldPositionX, miniTileWorldPositionY};
            glm::vec2 miniTileSize(miniWidth - gap, miniHeight - gap);

            auto entity = registry.create();
            registry.emplace<RenderingInfo>(
                entity, glm::vec2(miniWidth, miniHeight), tilesetTexture, miniTextureSrcRect);
            auto tilePhysicsBody = box2dBodyCreator.CreatePhysicsBody(entity, miniTileWorldPosition, miniTileSize);

            // Update level bounds.
            const b2Vec2& bodyPosition = tilePhysicsBody->GetBody()->GetPosition();
            auto& levelBounds = gameState.levelOptions.levelBox2dBounds;
            levelBounds.min = Vec2Min(levelBounds.min, bodyPosition);
            levelBounds.max = Vec2Max(levelBounds.max, bodyPosition);

            // Apply randomly: static/dynamic body.
            tilePhysicsBody->GetBody()->SetType(
                utils::randomTrue(gameState.levelOptions.dynamicBodyProbability) ? b2_dynamicBody : b2_staticBody);

            registry.emplace<PhysicsInfo>(entity, tilePhysicsBody);

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
            mapFilepath.parent_path() / mapJson["tilesets"][0]["source"].get<std::string>();
        std::ifstream tilesetFile(tilesetJsonPath);
        if (!tilesetFile.is_open())
            throw std::runtime_error("Failed to open tileset file");

        nlohmann::json tilesetJson;
        tilesetFile >> tilesetJson;
        tilesetPath = mapFilepath.parent_path() / tilesetJson["image"].get<std::string>();
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
        tilesetPath = mapFilepath.parent_path() / mapJson["tilesets"][0]["image"].get<std::string>();
    }
    else
    {
        throw std::runtime_error("[ReadPathToTileset] Failed to read path to tileset");
    }

    return tilesetPath;
};