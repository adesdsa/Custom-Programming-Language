#include "includes/shared/utils/system_utils.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
    #include <climits>
#endif

namespace SystemUtils
{
    std::filesystem::path GetExecutableDir()
    {
        #ifdef _WIN32
            wchar_t buf[MAX_PATH];
            GetModuleFileNameW(nullptr, buf, MAX_PATH);
            return std::filesystem::path(buf).parent_path();
        #else
            char buf[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
            return std::filesystem::path(
                std::string(buf, (count > 0) ? static_cast<size_t>(count) : 0)).parent_path();
        #endif
    }
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
        namespace fs = std::filesystem;

        // Try relative to the running executable (installed / deployed builds)
        fs::path exeRelative = GetExecutableDir() / relativePath;
        if (fs::exists(exeRelative))
        {
            return exeRelative;
        }

        // Fall back to PROJECT_ROOT (development builds)
        #ifdef _WIN32
                return fs::path(PROJECT_ROOT) / relativePath;
        #else
                return fs::path(PROJECT_ROOT) / fs::path(
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(relativePath));
        #endif
    }

    bool IsDirectoryWritable(const std::filesystem::path& dir)
    {
        if (!std::filesystem::exists(dir)) return false;

        std::filesystem::path probe = dir / ".picoc_write_test";
        std::ofstream ofs(probe);
        if (!ofs.is_open()) return false;

        ofs.close();
        std::filesystem::remove(probe);
        return true;
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
