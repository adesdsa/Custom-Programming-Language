#pragma once

#include <string>
#include <vector>
#include <optional>

#include "includes/shared/common/common_consts.h"
#include "includes/frontend/semantic_analysis/var_metadata.h"

struct BuiltinFunction
{
    std::string name;
    Type returnType;
    std::vector<VarMetaData> params;
};

struct BuiltinVariable
{
    std::string name;
    Type type;
    AccessRights access;
};

struct BuiltinLibrary
{
    std::string name;
    std::vector<BuiltinFunction> functions;
    std::vector<BuiltinVariable> variables;

    const BuiltinFunction* FindFunction(const std::string& funcName) const;
    const BuiltinVariable* FindVariable(const std::string& varName) const;
};

class BuiltinLibraryRegistry
{
public:
    static const BuiltinLibrary* GetLibrary(const std::string& name);
    static bool IsBuiltinLibrary(const std::string& name);
    static std::string GetObjPath(const std::string& libName);

private:
    static const std::vector<BuiltinLibrary>& GetAll();
};
