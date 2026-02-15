#include "includes/shared/utils/system_utils.h"

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
                return std::filesystem::path(PROJECT_ROOT) / relativePath; // PROJECT_ROOT is wstring
        #else
                return std::filesystem::path(PROJECT_ROOT) / std::filesystem::path(
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(relativePath));
        #endif
    }

    int RunProcess(const std::string& command, std::string& output)
    {
        output.clear();

        // redirect stderr to stdout so we capture everything
        std::string fullCommand = command + " 2>&1";

        #ifdef _WIN32
            FILE* pipe = _popen(fullCommand.c_str(), "r");
        #else
            FILE* pipe = popen(fullCommand.c_str(), "r");
        #endif

        if (!pipe)
        {
            output = "Failed to launch process: " + command;
            return -1;
        }

        std::array<char, 256> buffer{};
        while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        {
            output += buffer.data();
        }

        #ifdef _WIN32
            int exitCode = _pclose(pipe);
        #else
            int exitCode = pclose(pipe);
        #endif

        return exitCode;
    }
}
