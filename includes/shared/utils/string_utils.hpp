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

    /// @brief Convert a std::wstring to std::string (ASCII-safe).
    inline std::string ToNarrow(const std::wstring& wide)
    {
        std::string result;
        result.reserve(wide.size());
        for (wchar_t wc : wide)
        {
            result.push_back(static_cast<char>(wc));
        }
        return result;
    }

    /// @brief Check if a string starts with a given prefix.
    inline bool StartsWith(const std::string& str, const std::string& prefix)
    {
        return str.size() >= prefix.size() &&
               str.compare(0, prefix.size(), prefix) == 0;
    }

    /// @brief Check if a string ends with a given suffix.
    inline bool EndsWith(const std::string& str, const std::string& suffix)
    {
        return str.size() >= suffix.size() &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}

