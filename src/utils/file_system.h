#pragma once
#include <string>

namespace utils
{

/**
 * Checks if the specified file has changed since the last time it was checked.
 *
 * @param filename The name of the file to check.
 * @return True if the file has changed, false otherwise.
 */
bool FileChangedSinceLastCheck(std::string filename);

} // namespace utils