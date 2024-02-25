#include <filesystem>
#include <unordered_map>
#include <utils/file_system.h>

namespace utils
{

bool FileChangedSinceLastCheck(const std::filesystem::path& filename)
{
    auto absPath = std::filesystem::absolute(filename).string();

    static std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> fileTimes;

    std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(absPath);

    if (fileTimes.find(absPath) == fileTimes.end())
    {
        fileTimes[absPath] = writeTime;
        return false;
    }
    else if (fileTimes[absPath] != writeTime)
    {
        fileTimes[absPath] = writeTime;
        return true;
    }

    return false;
}

} // namespace utils