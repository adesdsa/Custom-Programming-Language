#include "includes/shared/common/builtin_library.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/shared/utils/system_utils.h"

const BuiltinFunction* BuiltinLibrary::FindFunction(const std::string& funcName) const
{
    for (const auto& func : functions)
    {
        if (func.name == funcName) return &func;
    }
    return nullptr;
}

const BuiltinVariable* BuiltinLibrary::FindVariable(const std::string& varName) const
{
    for (const auto& var : variables)
    {
        if (var.name == varName) return &var;
    }
    return nullptr;
}

/**
 * Interface for string library, contains the built-in functions + global vars
*/
static BuiltinLibrary BuildStringLibrary()
{
    BuiltinLibrary lib;
    lib.name = "string";

    auto mkParam = [](const std::string& name, Type type) {
        return VarMetaData(name, type, AccessRights::DEFAULT_ACCESS, Scope::LET, 0);
    };

    lib.functions = {
        { "str_concat",  Type::STRING_TYPE, { mkParam("a", Type::STRING_TYPE), mkParam("b", Type::STRING_TYPE) } },
        { "str_from_char", Type::STRING_TYPE, { mkParam("c", Type::CHAR_TYPE) } },
        { "str_cmp",     Type::INT_TYPE,    { mkParam("a", Type::STRING_TYPE), mkParam("b", Type::STRING_TYPE) } },
        { "str_len",     Type::INT_TYPE,    { mkParam("s", Type::STRING_TYPE) } },
        { "substr",      Type::STRING_TYPE, { mkParam("s", Type::STRING_TYPE), mkParam("start", Type::INT_TYPE), mkParam("length", Type::INT_TYPE) } },
        { "substr_step", Type::STRING_TYPE, { mkParam("s", Type::STRING_TYPE), mkParam("start", Type::INT_TYPE), mkParam("end", Type::INT_TYPE), mkParam("step", Type::INT_TYPE) } },
        { "char_at",     Type::CHAR_TYPE,   { mkParam("s", Type::STRING_TYPE), mkParam("index", Type::INT_TYPE) } },
        { "set_char_at", Type::VOID_TYPE,   { mkParam("s", Type::STRING_TYPE), mkParam("index", Type::INT_TYPE), mkParam("c", Type::CHAR_TYPE) } },
        { "str_slice_set", Type::VOID_TYPE, { mkParam("s", Type::STRING_TYPE), mkParam("start", Type::INT_TYPE), mkParam("end", Type::INT_TYPE), mkParam("step", Type::INT_TYPE), mkParam("replacement", Type::STRING_TYPE) } },
        { "str_find",       Type::INT_TYPE,  { mkParam("s", Type::STRING_TYPE), mkParam("ch", Type::CHAR_TYPE) } },
        { "str_find_first_of", Type::INT_TYPE, { mkParam("s", Type::STRING_TYPE), mkParam("ch", Type::CHAR_TYPE) } },
        { "str_find_last_of",  Type::INT_TYPE, { mkParam("s", Type::STRING_TYPE), mkParam("ch", Type::CHAR_TYPE) } },
        { "str_back",  Type::CHAR_TYPE, { mkParam("s", Type::STRING_TYPE) } },
        { "str_front", Type::CHAR_TYPE, { mkParam("s", Type::STRING_TYPE) } }
    };

    lib.variables = {
        { "EOS", Type::INT_TYPE, AccessRights::CONST_ACCESS }
    };

    return lib;
}

static BuiltinLibrary BuildRuntimeLibrary()
{
    BuiltinLibrary lib;
    lib.name = "runtime";

    auto mkParam = [](const std::string& name, Type type) {
        return VarMetaData(name, type, AccessRights::DEFAULT_ACCESS, Scope::LET, 0);
    };

    lib.functions = {
        { "bounds_check", Type::VOID_TYPE, { mkParam("index", Type::INT_TYPE), mkParam("size", Type::INT_TYPE) } }
    };

    return lib;
}

const std::vector<BuiltinLibrary>& BuiltinLibraryRegistry::GetAll()
{
    static std::vector<BuiltinLibrary> libs = { BuildStringLibrary(), BuildRuntimeLibrary() };
    return libs;
}

const BuiltinLibrary* BuiltinLibraryRegistry::GetLibrary(const std::string& name)
{
    for (const auto& lib : GetAll())
    {
        if (lib.name == name) return &lib;
    }
    return nullptr;
}

bool BuiltinLibraryRegistry::IsBuiltinLibrary(const std::string& name)
{
    return GetLibrary(name) != nullptr;
}

std::string BuiltinLibraryRegistry::GetObjPath(const std::string& libName)
{
    std::string target;
    if (TargetArchitecture::GetOS() == OS::Windows)
        target = TargetArchitecture::Is64Bit() ? "win64" : "win32";
    else
        target = TargetArchitecture::Is64Bit() ? "linux64" : "linux32";

    std::string ext = (TargetArchitecture::GetOS() == OS::Windows) ? ".obj" : ".o";
    std::string relativePath = "lib/" + libName + "_" + target + ext;

    auto resolved = SystemUtils::BuildRelativePath(
        std::wstring(relativePath.begin(), relativePath.end()));
    return resolved.string();
}
