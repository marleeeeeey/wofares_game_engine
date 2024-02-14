#include <filesystem>
#include <unordered_map>
#include <utils/file_system.h>

namespace utils
{

bool FileChangedSinceLastCheck(std::string filename)
{
    filename = std::filesystem::absolute(filename).string();

    static std::unordered_map<std::string, std::filesystem::file_time_type> fileTimes;

    std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(filename);

    if (fileTimes.find(filename) == fileTimes.end())
    {
        fileTimes[filename] = writeTime;
        return false;
    }
    else if (fileTimes[filename] != writeTime)
    {
        fileTimes[filename] = writeTime;
        return true;
    }

    return false;
}

} // namespace utils