#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "includes/shared/common/common_consts.h"

struct ExportedVar 
{
    std::string name;
    Type type;
    AccessRights accessRights;
};

struct ExportedFunc 
{
    std::string name;
    Type returnType;
    std::vector<ExportedVar> params;
};

struct FileExportTable 
{
    std::string filePath;
    std::string fileName;
    std::vector<ExportedFunc> functions;
    std::vector<ExportedVar> exportedVars;
};

/// @brief Holds all exported variables and functions from files 
/// that can be shared via import
class ProjectSymbolTable 
{
    std::unordered_map<std::string, FileExportTable> files;

public:
    const FileExportTable* Resolve(const std::string& fileName) const;
};
