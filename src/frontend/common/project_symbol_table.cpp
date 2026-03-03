#include "includes/frontend/common/project_symbol_table.h"

const FileExportTable* ProjectSymbolTable::Resolve(const std::string& fileName) const
{
    auto it = this->files.find(fileName);

    if (it != this->files.end()) return &it->second;
    else return nullptr;
}
