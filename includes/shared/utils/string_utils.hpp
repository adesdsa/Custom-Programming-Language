#pragma once

#include <string>
#include <cstring>

namespace StringUtils
{
    /// @brief Convert a narrow C-string to std::wstring.
    inline std::wstring ToWide(const char* narrow)
    {
        return std::wstring(narrow, narrow + std::strlen(narrow));
    }

    /// @brief Check if a string starts with a given prefix.
    inline bool StartsWith(const std::string& str, const std::string& prefix)
    {
        return str.size() >= prefix.size() &&
               str.compare(0, prefix.size(), prefix) == 0;
    }
}

