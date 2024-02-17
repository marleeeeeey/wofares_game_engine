#include "map_loader.h"
#include <SDL_image.h>
#include <box2d/b2_math.h>
#include <ecs/systems/details/physics_body_creator.h>
#include <fstream>
#include <my_common_cpp_utils/MathUtils.h>
#include <utils/glm_box2d_conversions.h>
#include <utils/texture_process.h>

MapLoader::MapLoader(const std::string& filename, entt::registry& registry, SDL_Renderer* renderer)
  : registry(registry), gameState(registry.get<GameState>(registry.view<GameState>().front())),
    objectsFactory(registry), coordinatesTransformer(registry)
{
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");

    nlohmann::json json;
    file >> json;

    // Calc path to tileset image.
    std::string tilesetPath = filename;
    size_t found = tilesetPath.find_last_of("/\\");
    if (found != std::string::npos)
        tilesetPath = tilesetPath.substr(0, found + 1);
    tilesetPath += json["tilesets"][0]["image"];

    // Check if the tileset texture file exists.
    std::ifstream tilesetFile(tilesetPath);
    if (!tilesetFile.is_open())
        throw std::runtime_error(MY_FMT("Failed to open tileset file {}", tilesetPath));

    // Load the tileset texture.
    if (gameState.levelOptions.preventCreationInvisibleTiles)
        tilesetTexture = LoadTextureWithStreamingAccess(renderer, tilesetPath);
    else
        tilesetTexture = LoadTexture(renderer, tilesetPath);

    // Assume all tiles are of the same size.
    tileWidth = json["tilewidth"];
    tileHeight = json["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    colAndRowNumber = gameState.levelOptions.miniTileResolution;
    miniWidth = tileWidth / colAndRowNumber;
    miniHeight = tileHeight / colAndRowNumber;

    // Iterate over each tile layer.
    for (const auto& layer : json["layers"])
    {
        if (layer["type"] == "tilelayer")
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
        MY_LOG_FMT(warn, "No tiles were created during map loading {}", filename);
        if (invisibleTilesNumber > 0)
            MY_LOG(warn, "All tiles are invisible");
    }
}

void MapLoader::ParseTileLayer(const nlohmann::json& layer)
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

void MapLoader::ParseObjectLayer(const nlohmann::json& layer)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    for (const auto& object : layer["objects"])
    {
        if (object["type"] == "PlayerPosition")
        {
            auto playerSdlWorldPos = glm::vec2(object["x"], object["y"]);
            objectsFactory.createPlayer(playerSdlWorldPos);
        }
    }
}

void MapLoader::CalculateLevelBoundsWithBufferZone()
{
    auto& lb = gameState.levelOptions.levelBox2dBounds;
    auto& bz = gameState.levelOptions.bufferZone;
    MY_LOG_FMT(info, "Level bounds: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
    lb.min -= bz;
    lb.max += bz;
    MY_LOG_FMT(
        info, "Level bounds with buffer zone: min: ({}, {}), max: ({}, {})", lb.min.x, lb.min.y, lb.max.x, lb.max.y);
}

void MapLoader::ParseTile(int tileId, int layerCol, int layerRow)
{
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.physicsOptions.gapBetweenPhysicalAndVisual;

    SDL_Rect textureSrcRect = CalculateSrcRect(tileId, tileWidth, tileHeight, tilesetTexture);

    // Create entities for each mini tile inside the tile.
    for (int miniRow = 0; miniRow < colAndRowNumber; ++miniRow)
    {
        for (int miniCol = 0; miniCol < colAndRowNumber; ++miniCol)
        {
            SDL_Rect miniTextureSrcRect{
                textureSrcRect.x + miniCol * miniWidth, textureSrcRect.y + miniRow * miniHeight, miniWidth, miniHeight};

            if (gameState.levelOptions.preventCreationInvisibleTiles &&
                IsTileInvisible(tilesetTexture, miniTextureSrcRect))
            {
                invisibleTilesNumber++;
                continue;
            }

            float miniTileWorldPositionX = layerCol * tileWidth + miniCol * miniWidth;
            float miniTileWorldPositionY = layerRow * tileHeight + miniRow * miniHeight;
            glm::vec2 miniTileWorldPosition{miniTileWorldPositionX, miniTileWorldPositionY};
            glm::vec2 miniTileSize(miniWidth - gap, miniHeight - gap);

            auto entity = registry.create();
            registry.emplace<RenderingInfo>(
                entity, glm::vec2(miniWidth, miniHeight), tilesetTexture, miniTextureSrcRect);
            auto tilePhysicsBody = CreateStaticPhysicsBody(
                entity, coordinatesTransformer, physicsWorld, miniTileWorldPosition, miniTileSize);

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
