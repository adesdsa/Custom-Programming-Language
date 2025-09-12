#pragma once

#include <string>

namespace FileUtils
{
    // Return the content of the file.
    // @param path: Path to target file.
    // @return: Content of the file.
    std::string ReadFile(const std::string& path);
}