#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

struct LevelInfo
{
    std::string name;
    std::filesystem::path tiledMapPath;
    std::filesystem::path backgroundPath;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LevelInfo, name, tiledMapPath, backgroundPath)
};
