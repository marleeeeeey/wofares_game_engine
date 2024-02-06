#include "load_map_systems.h"
#include <ecs/components/all_components.h>
#include <fstream>
#include <my_common_cpp_utils/Logger.h>
#include <nlohmann/json.hpp>

void LoadMap(entt::registry& registry, const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open map file");

    nlohmann::json json;
    file >> json;

    // Assume all tiles are of the same size and the map is not infinite
    int tileWidth = json["tilewidth"];
    int tileHeight = json["tileheight"];
    glm::vec2 tileSize{tileWidth, tileHeight};

    // Iterate over each tile layer
    size_t createdTiles = 0;
    for (const auto& layer : json["layers"])
    {
        if (layer["type"] == "tilelayer")
        {
            int width = layer["width"];
            int height = layer["height"];
            const auto& tiles = layer["data"];

            // Create entities for each tile
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int tileId = tiles[x + y * width];
                    if (tileId > 0)
                    { // Skip empty tiles
                        auto entity = registry.create();
                        registry.emplace<Position>(entity, glm::vec2(x * tileWidth, y * tileHeight));
                        registry.emplace<SizeComponent>(entity, tileSize);
                        createdTiles++;
                        // TODO Additional components can be added here, such as tile type if needed
                    }
                }
            }
        }
    }

    if (createdTiles == 0)
        throw std::runtime_error(MY_FMT("No tiles were created during map loading {}", filename));
}
