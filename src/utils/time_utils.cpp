#include "time_utils.h"
#include <chrono>

namespace utils
{

std::time_t CurrentTimeSeconds()
{
    const auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
}

std::chrono::milliseconds CurrentTimeMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}
} // namespace utils