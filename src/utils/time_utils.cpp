#include "../../includes/utils/time_utils.h"

namespace TimeUtils
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
}
