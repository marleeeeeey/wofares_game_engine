#include "load_map_systems.h"
#include "SDL_image.h"
#include "utils/sdl_RAII.h"
#include <ecs/components/all_components.h>
#include <fstream>
#include <memory>
#include <my_common_cpp_utils/Logger.h>
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

                    auto entity = registry.create();
                    registry.emplace<Position>(entity, glm::vec2(layerCol * tileWidth, layerRow * tileHeight));
                    registry.emplace<SizeComponent>(entity, glm::vec2(tileWidth, tileHeight));
                    registry.emplace<TileInfo>(entity, TileInfo{});
                    // Calculate srcRect for Renderable component.
                    SDL_Rect srcRect = CalculateSrcRect(tileId, tileWidth, tileHeight, tilesetTexture);
                    registry.emplace<Renderable>(entity, Renderable{tilesetTexture, srcRect});
                    // TODO Additional components can be added here, such as tile type if needed.
                    createdTiles++;
                }
            }
        }
    }

    if (createdTiles == 0)
        throw std::runtime_error(MY_FMT("No tiles were created during map loading {}", filename));
}
