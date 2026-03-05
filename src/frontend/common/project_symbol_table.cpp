#include "includes/frontend/common/project_symbol_table.h"

void ProjectSymbolTable::Register(const std::string& fileName, FileExportTable table)
{
    this->files[fileName] = std::move(table);
}

const FileExportTable* ProjectSymbolTable::Resolve(const std::string& fileName) const
{
    auto it = this->files.find(fileName);

    if (it != this->files.end()) return &it->second;
    else return nullptr;
}
