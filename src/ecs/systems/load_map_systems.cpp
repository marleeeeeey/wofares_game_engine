#include "load_map_systems.h"
#include "SDL_image.h"
#include "box2d/b2_body.h"
#include "glm/fwd.hpp"
#include "utils/sdl_RAII.h"
#include <ecs/components/all_components.h>
#include <fstream>
#include <memory>
#include <my_common_cpp_utils/Logger.h>
#include <my_common_cpp_utils/MathUtils.h>
#include <nlohmann/json.hpp>

namespace
{

std::shared_ptr<Texture> LoadTexture(SDL_Renderer* renderer, const std::string& filePath)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, filePath.c_str());

    if (texture == nullptr)
        throw std::runtime_error("Failed to load texture");

    return std::make_shared<Texture>(texture);
}

SDL_Rect CalculateSrcRect(int tileId, int tileWidth, int tileHeight, std::shared_ptr<Texture> texture)
{
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture->get(), nullptr, nullptr, &textureWidth, &textureHeight);

    int tilesPerRow = textureWidth / tileWidth;
    tileId -= 1; // Adjust tileId to match 0-based indexing.

    SDL_Rect srcRect;
    srcRect.x = (tileId % tilesPerRow) * tileWidth;
    srcRect.y = (tileId / tilesPerRow) * tileHeight;
    srcRect.w = tileWidth;
    srcRect.h = tileHeight;

    return srcRect;
}
} // namespace

void UnloadMap(entt::registry& registry)
{
    // Remove all entities that have a TileInfo component.
    auto view = registry.view<TileInfo>();
    for (auto entity : view)
    {
        registry.destroy(entity);
    }
}

void LoadMap(entt::registry& registry, SDL_Renderer* renderer, const std::string& filename)
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
    auto tilesetTexture = LoadTexture(renderer, tilesetPath);
    int firstGid = json["tilesets"][0]["firstgid"];

    // Assume all tiles are of the same size.
    int tileWidth = json["tilewidth"];
    int tileHeight = json["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    const int colAndRowNumber = 1;
    const int miniWidth = tileWidth / colAndRowNumber;
    const int miniHeight = tileHeight / colAndRowNumber;

    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;

    // Iterate over each tile layer.
    size_t createdTiles = 0;
    for (const auto& layer : json["layers"])
    {
        if (layer["type"] == "tilelayer")
        {
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

                    SDL_Rect textureSrcRect = CalculateSrcRect(tileId, tileWidth, tileHeight, tilesetTexture);

                    // Create entities for each mini tile inside the tile.
                    for (int miniRow = 0; miniRow < colAndRowNumber; ++miniRow)
                    {
                        for (int miniCol = 0; miniCol < colAndRowNumber; ++miniCol)
                        {
                            SDL_Rect miniTextureSrcRect{
                                textureSrcRect.x + miniCol * miniWidth, textureSrcRect.y + miniRow * miniHeight,
                                miniWidth, miniHeight};

                            glm::u32vec2 miniTileWorldPosition(
                                layerCol * tileWidth + miniCol * miniWidth,
                                layerRow * tileHeight + miniRow * miniHeight);
                            auto entity = registry.create();
                            registry.emplace<Position>(entity, miniTileWorldPosition);
                            registry.emplace<SizeComponent>(entity, glm::vec2(miniWidth, miniHeight));
                            registry.emplace<TileInfo>(entity, tilesetTexture, miniTextureSrcRect);

                            // *************************************** PHYSICS ***************************************

                            b2BodyDef bodyDef;
                            bodyDef.type = utils::randomBool() ? b2_dynamicBody : b2_staticBody;
                            bodyDef.position.Set(miniTileWorldPosition.x, miniTileWorldPosition.y);
                            b2Body* body = physicsWorld->CreateBody(&bodyDef);

                            b2PolygonShape shape;
                            shape.SetAsBox(miniWidth / 2.0, miniHeight / 2.0);

                            b2FixtureDef fixtureDef;
                            fixtureDef.shape = &shape;
                            fixtureDef.density = 1.0f; // Density to calculate mass
                            fixtureDef.friction = 0.3f; // Friction to apply to the body
                            body->CreateFixture(&fixtureDef);

                            registry.emplace<Box2dObject>(
                                entity, std::make_shared<Box2dObjectRAII>(body, physicsWorld));

                            // ***************************************************************************************

                            createdTiles++;
                        }
                    }
                }
            }
        }
    }

    if (createdTiles == 0)
        throw std::runtime_error(MY_FMT("No tiles were created during map loading {}", filename));
}
