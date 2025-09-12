#include "../../includes/utils/file_utils.h"

#include <sstream>
#include <iostream>
#include <fstream>


namespace FileUtils
{
    std::string ReadFile(const std::string& path)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}