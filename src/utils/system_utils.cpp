#include "includes/utils/system_utils.h"

namespace SystemUtils
{
    std::string GetFullCurrentTime()
    {
        // get current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // convert to local time
        std::tm localTime{};

        #ifdef _WIN32
            localtime_s(&localTime, &now_c); // Windows

        #else
            localtime_r(&now_c, &localTime); // Linux / macOS
        #endif

        // Format it
        std::ostringstream oss;
        oss << std::put_time(&localTime, "%d/%m/%Y %H:%M:%S");
        return oss.str();
    }

    const std::filesystem::path BuildRelativePath(const std::wstring& relativePath)
    {
#ifdef _WIN32
        return std::filesystem::path(PROJECT_ROOT) / relativePath;
#else
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        std::string utf8Root = conv.to_bytes(PROJECT_ROOT);
        std::string utf8Rel = conv.to_bytes(relativePath);
        return std::filesystem::path(utf8Root) / std::filesystem::path(utf8Rel);
#endif
    }
}
