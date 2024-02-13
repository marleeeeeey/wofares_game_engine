#include "load_map_systems.h"
#include <SDL_image.h>
#include <box2d/b2_body.h>
#include <cstddef>
#include <ecs/components/all_components.h>
#include <fstream>
#include <glm/fwd.hpp>
#include <memory>
#include <my_common_cpp_utils/Logger.h>
#include <my_common_cpp_utils/MathUtils.h>
#include <nlohmann/json.hpp>
#include <utils/glm_box2d_conversions.h>
#include <utils/globals.h>
#include <utils/sdl_RAII.h>

namespace
{

/**
 * Checks if the given tileset texture has an opaque pixel within the specified source rectangle.
 * Note that in order to use SDL_LockTexture and SDL_UnlockTexture, the texture must be created with
 * SDL_TEXTUREACCESS_STREAMING access.
 *
 * @param tilesetTexture The tileset texture to check.
 * @param miniTextureSrcRect The source rectangle within the tileset texture.
 * @return True if the tileset texture has an opaque pixel within the source rectangle, false otherwise.
 */
bool HasOpaquePixel(std::shared_ptr<Texture> tilesetTexture, const SDL_Rect& miniTextureSrcRect)
{
    Uint32* pixels;
    int pitch; // The length of a row of pixels in bytes.

    if (SDL_LockTexture(tilesetTexture->get(), &miniTextureSrcRect, (void**)&pixels, &pitch) != 0)
    {
        MY_LOG_FMT(
            warn,
            "SDL_LockTexture failed. Check that you use function LoadTextureWithStreamingAccess to load texture. Error: {}",
            SDL_GetError());
        return false;
    }

    bool hasOpaquePixel = false;
    for (int row = 0; row < miniTextureSrcRect.h; ++row)
    {
        for (int col = 0; col < miniTextureSrcRect.w; ++col)
        {
            static const int pixelSize = 4; // 4 bytes per pixel (ABGR).
            // TODO: here is a bug. pixel is always zero. Probably the issue in LoadTextureWithStreamingAccess.
            Uint32 pixel = pixels[row * (pitch / pixelSize) + col];
            Uint8 alpha = pixel & 0xFF; // Correct for SDL_PIXELFORMAT_ABGR8888
            // MY_LOG_FMT(info, "Alpha: {}, Pixel: {}, Pitch: {}, col: {}, row: {}", alpha, pixel, pitch, col, row);
            if (alpha > 0)
            {
                hasOpaquePixel = true;
                break;
            }
        }
        if (hasOpaquePixel)
        {
            break;
        }
    }

    SDL_UnlockTexture(tilesetTexture->get());
    return hasOpaquePixel;
}

/**
 * Loads a texture from the specified file path.
 *
 * @param renderer The SDL renderer used to load the texture.
 * @param filePath The file path of the texture to be loaded.
 * @return A shared pointer to the loaded texture.
 */
std::shared_ptr<Texture> LoadTexture(SDL_Renderer* renderer, const std::string& filePath)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, filePath.c_str());

    if (texture == nullptr)
        throw std::runtime_error("Failed to load texture");

    return std::make_shared<Texture>(texture);
}

/**
 * @brief Loads a texture with streaming access.
 *
 * This function loads a texture from the specified file path using SDL_Renderer with streaming access.
 *
 * @param renderer The SDL_Renderer pointer.
 * @param filePath The file path of the texture to load.
 * @return A shared pointer to the loaded texture.
 */
std::shared_ptr<Texture> LoadTextureWithStreamingAccess(SDL_Renderer* renderer, const std::string& filePath)
{
    // Step 1. Load image into SDL_Surface.
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface)
        throw std::runtime_error(MY_FMT("Failed to load image {}. Error: {}", filePath, IMG_GetError()));

    MY_LOG_FMT(
        info, "Surface format: {}, w: {}, h: {}", SDL_GetPixelFormatName(surface->format->format), surface->w,
        surface->h);

    // Step 2: Create a texture with the SDL_TEXTUREACCESS_STREAMING flag.
    SDL_Texture* texture =
        SDL_CreateTexture(renderer, surface->format->format, SDL_TEXTUREACCESS_STREAMING, surface->w, surface->h);
    if (!texture)
    {
        SDL_FreeSurface(surface);
        throw std::runtime_error(
            MY_FMT("Failed to create streaming texture for image {}. Error: {}", filePath, SDL_GetError()));
    }

    // Step 3. Copy pixel data from the surface to the texture.
    if (SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch) != 0)
    {
        MY_LOG_FMT(warn, "SDL_UpdateTexture failed. Error: {}", SDL_GetError());
    }

    // Step 4. Free the surface.
    SDL_FreeSurface(surface);

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

std::shared_ptr<Box2dObjectRAII> CreateStaticPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::u32vec2& sdlPos, const glm::u32vec2& sdlSize)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(sdlPos.x * sdlToBox2D, sdlPos.y * sdlToBox2D);
    b2Body* body = physicsWorld->CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(sdlSize.x / 2.0 * sdlToBox2D, sdlSize.y / 2.0 * sdlToBox2D);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 10.0f; // Density to calculate mass
    fixtureDef.friction = 0.3f; // Friction to apply to the body
    body->CreateFixture(&fixtureDef);

    return std::make_shared<Box2dObjectRAII>(body, physicsWorld);
}

std::shared_ptr<Box2dObjectRAII> CreateDynamicPhysicsBody(
    std::shared_ptr<b2World> physicsWorld, const glm::u32vec2& sdlPos, const glm::u32vec2& sdlSize)
{
    auto staticBody = CreateStaticPhysicsBody(physicsWorld, sdlPos, sdlSize);
    staticBody->GetBody()->SetType(b2_dynamicBody);
    return staticBody;
}

} // namespace

void UnloadMap(entt::registry& registry)
{
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    gameState.levelInfo = {};

    // Remove all entities that have a TileInfo component.
    for (auto entity : registry.view<TileInfo>())
        registry.destroy(entity);

    // Remove all entities that have a PhysicalBody component.
    for (auto entity : registry.view<PhysicalBody>())
        registry.destroy(entity);

    if (Box2dObjectRAII::GetBodyCounter() != 0)
        MY_LOG_FMT(warn, "There are still {} Box2D bodies in the memory", Box2dObjectRAII::GetBodyCounter());
    else
        MY_LOG(debug, "All Box2D bodies were destroyed");
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

    // Get the physics world.
    auto& gameState = registry.get<GameState>(registry.view<GameState>().front());
    auto physicsWorld = gameState.physicsWorld;
    auto gap = gameState.gapBetweenPhysicalAndVisual;

    // Load the tileset texture.
    std::shared_ptr<Texture> tilesetTexture;
    if (gameState.preventCreationInvisibleTiles)
        tilesetTexture = LoadTextureWithStreamingAccess(renderer, tilesetPath);
    else
        tilesetTexture = LoadTexture(renderer, tilesetPath);

    int firstGid = json["tilesets"][0]["firstgid"];

    // Assume all tiles are of the same size.
    int tileWidth = json["tilewidth"];
    int tileHeight = json["tileheight"];

    // Calculate mini tile size: 4x4 mini tiles in one big tile.
    const int colAndRowNumber = gameState.miniTileResolution;
    const int miniWidth = tileWidth / colAndRowNumber;
    const int miniHeight = tileHeight / colAndRowNumber;

    // Iterate over each tile layer.
    size_t createdTiles = 0;
    size_t invisibleTilesNumber = 0;

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

                            if (gameState.preventCreationInvisibleTiles &&
                                !HasOpaquePixel(tilesetTexture, miniTextureSrcRect))
                            {
                                invisibleTilesNumber++;
                                continue;
                            }

                            glm::u32vec2 miniTileWorldPosition(
                                layerCol * tileWidth + miniCol * miniWidth,
                                layerRow * tileHeight + miniRow * miniHeight);
                            auto entity = registry.create();
                            registry.emplace<SizeComponent>(entity, glm::vec2(miniWidth, miniHeight));
                            registry.emplace<TileInfo>(entity, tilesetTexture, miniTextureSrcRect);

                            glm::u32vec2 miniTileSize(miniWidth - gap, miniHeight - gap);

                            auto tilePhysicsBody =
                                CreateStaticPhysicsBody(physicsWorld, miniTileWorldPosition, miniTileSize);

                            // Update level bounds.
                            const auto& bodyPosition = tilePhysicsBody->GetBody()->GetPosition();
                            auto& levelBounds = gameState.levelInfo.levelBounds;
                            levelBounds.min = Vec2Min(levelBounds.min, bodyPosition);
                            levelBounds.max = Vec2Max(levelBounds.max, bodyPosition);

                            // Apply randomly: static/dynamic body.
                            tilePhysicsBody->GetBody()->SetType(
                                utils::randomTrue(gameState.dynamicBodyProbability) ? b2_dynamicBody : b2_staticBody);

                            registry.emplace<PhysicalBody>(entity, tilePhysicsBody);

                            createdTiles++;
                        }
                    }
                }
            }
        }
        else if (layer["type"] == "objectgroup")
        {
            for (const auto& object : layer["objects"])
            {
                if (object["type"] == "PlayerPosition")
                {
                    auto entity = registry.create();
                    auto playerSize = glm::u32vec2(10, 10);
                    registry.emplace<SizeComponent>(entity, playerSize);
                    registry.emplace<PlayerNumber>(entity);

                    auto playerPhysicsBody = CreateDynamicPhysicsBody(
                        physicsWorld, glm::u32vec2(object["x"], object["y"]), playerSize - glm::u32vec2{gap, gap});
                    registry.emplace<PhysicalBody>(entity, playerPhysicsBody);
                }
            }
        }
    }

    // Add buffer zone to the level bounds.
    auto& levelBounds = gameState.levelInfo.levelBounds;
    auto& bufferZone = gameState.levelInfo.bufferZone;
    MY_LOG_FMT(
        info, "Level bounds: min: ({}, {}), max: ({}, {})", levelBounds.min.x, levelBounds.min.y, levelBounds.max.x,
        levelBounds.max.y);
    levelBounds.min -= bufferZone;
    levelBounds.max += bufferZone;
    MY_LOG_FMT(
        info, "Level bounds with buffer zone: min: ({}, {}), max: ({}, {})", levelBounds.min.x, levelBounds.min.y,
        levelBounds.max.x, levelBounds.max.y);

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
