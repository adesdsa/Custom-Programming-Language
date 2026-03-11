#include "includes/shared/common/builtin_library.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/shared/utils/system_utils.h"

#include <filesystem>

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

     lib.functions = {
        { "bounds_check", Type::VOID_TYPE, { mkParam("index", Type::INT_TYPE), mkParam("size", Type::INT_TYPE) } }
    };
}

/**
 * Interface for math library, contains the built-in math functions
*/
static BuiltinLibrary BuildMathLibrary()
{
    BuiltinLibrary lib;
    lib.name = "math";

    auto mkParam = [](const std::string& name, Type type) {
        return VarMetaData(name, type, AccessRights::DEFAULT_ACCESS, Scope::LET, 0);
    };

    lib.functions = {
        { "sqrt", Type::DOUBLE_TYPE, { mkParam("x", Type::DOUBLE_TYPE) } },
        { "pow",  Type::DOUBLE_TYPE, { mkParam("x", Type::DOUBLE_TYPE), mkParam("y", Type::DOUBLE_TYPE) } },
        { "min",  Type::DOUBLE_TYPE, { mkParam("x", Type::DOUBLE_TYPE), mkParam("y", Type::DOUBLE_TYPE) } },
        { "max",  Type::DOUBLE_TYPE, { mkParam("x", Type::DOUBLE_TYPE), mkParam("y", Type::DOUBLE_TYPE) } },
    };

    return lib;
}

/// Array param: element type + arraySize=1 as sentinel for "array of this type"
static VarMetaData MkArrayParam(const std::string& name, Type elemType)
{
    return VarMetaData(name, elemType, AccessRights::DEFAULT_ACCESS, Scope::LET, 0, false, std::optional<size_t>(1));
}

static BuiltinLibrary BuildArrayLibrary()
{
    BuiltinLibrary lib;
    lib.name = "arraylib";

    auto mkParam = [](const std::string& name, Type type) {
        return VarMetaData(name, type, AccessRights::DEFAULT_ACCESS, Scope::LET, 0);
    };

    lib.functions = {
        // int[]
        { "front",  Type::INT_TYPE,    { MkArrayParam("arr", Type::INT_TYPE) } },
        { "back",   Type::INT_TYPE,    { MkArrayParam("arr", Type::INT_TYPE) } },
        { "len",    Type::INT_TYPE,    { MkArrayParam("arr", Type::INT_TYPE) } },
        { "find",   Type::INT_TYPE,    { MkArrayParam("arr", Type::INT_TYPE), mkParam("value", Type::INT_TYPE) } },
        { "find_first_of", Type::INT_TYPE, { MkArrayParam("arr", Type::INT_TYPE), mkParam("value", Type::INT_TYPE) } },
        { "find_last_of",  Type::INT_TYPE, { MkArrayParam("arr", Type::INT_TYPE), mkParam("value", Type::INT_TYPE) } },
        // string[]
        { "front_str",  Type::STRING_TYPE, { MkArrayParam("arr", Type::STRING_TYPE) } },
        { "back_str",   Type::STRING_TYPE, { MkArrayParam("arr", Type::STRING_TYPE) } },
        { "len_str",    Type::INT_TYPE,    { MkArrayParam("arr", Type::STRING_TYPE) } },
        { "find_str",   Type::INT_TYPE,    { MkArrayParam("arr", Type::STRING_TYPE), mkParam("value", Type::STRING_TYPE) } },
        { "find_first_of_str", Type::INT_TYPE, { MkArrayParam("arr", Type::STRING_TYPE), mkParam("value", Type::STRING_TYPE) } },
        { "find_last_of_str",  Type::INT_TYPE, { MkArrayParam("arr", Type::STRING_TYPE), mkParam("value", Type::STRING_TYPE) } },
        // double[]
        { "front_double",  Type::DOUBLE_TYPE, { MkArrayParam("arr", Type::DOUBLE_TYPE) } },
        { "back_double",   Type::DOUBLE_TYPE, { MkArrayParam("arr", Type::DOUBLE_TYPE) } },
        { "len_double",    Type::INT_TYPE,    { MkArrayParam("arr", Type::DOUBLE_TYPE) } },
        { "find_double",   Type::INT_TYPE,    { MkArrayParam("arr", Type::DOUBLE_TYPE), mkParam("value", Type::DOUBLE_TYPE) } },
        { "find_first_of_double", Type::INT_TYPE, { MkArrayParam("arr", Type::DOUBLE_TYPE), mkParam("value", Type::DOUBLE_TYPE) } },
        { "find_last_of_double",  Type::INT_TYPE, { MkArrayParam("arr", Type::DOUBLE_TYPE), mkParam("value", Type::DOUBLE_TYPE) } },
        // unsigned int[]
        { "front_uint",  Type::UNSIGNED_INT_TYPE, { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE) } },
        { "back_uint",   Type::UNSIGNED_INT_TYPE, { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE) } },
        { "len_uint",    Type::INT_TYPE,         { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE) } },
        { "find_uint",   Type::INT_TYPE,         { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE), mkParam("value", Type::UNSIGNED_INT_TYPE) } },
        { "find_first_of_uint", Type::INT_TYPE, { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE), mkParam("value", Type::UNSIGNED_INT_TYPE) } },
        { "find_last_of_uint",  Type::INT_TYPE, { MkArrayParam("arr", Type::UNSIGNED_INT_TYPE), mkParam("value", Type::UNSIGNED_INT_TYPE) } },
        // char[]
        { "front_char",  Type::CHAR_TYPE, { MkArrayParam("arr", Type::CHAR_TYPE) } },
        { "back_char",   Type::CHAR_TYPE, { MkArrayParam("arr", Type::CHAR_TYPE) } },
        { "len_char",    Type::INT_TYPE,   { MkArrayParam("arr", Type::CHAR_TYPE) } },
        { "find_char",   Type::INT_TYPE,   { MkArrayParam("arr", Type::CHAR_TYPE), mkParam("value", Type::CHAR_TYPE) } },
        { "find_first_of_char", Type::INT_TYPE, { MkArrayParam("arr", Type::CHAR_TYPE), mkParam("value", Type::CHAR_TYPE) } },
        { "find_last_of_char",  Type::INT_TYPE, { MkArrayParam("arr", Type::CHAR_TYPE), mkParam("value", Type::CHAR_TYPE) } },
        // bool[]
        { "front_bool",  Type::BOOL_TYPE, { MkArrayParam("arr", Type::BOOL_TYPE) } },
        { "back_bool",   Type::BOOL_TYPE, { MkArrayParam("arr", Type::BOOL_TYPE) } },
        { "len_bool",    Type::INT_TYPE,   { MkArrayParam("arr", Type::BOOL_TYPE) } },
        { "find_bool",   Type::INT_TYPE,   { MkArrayParam("arr", Type::BOOL_TYPE), mkParam("value", Type::BOOL_TYPE) } },
        { "find_first_of_bool", Type::INT_TYPE, { MkArrayParam("arr", Type::BOOL_TYPE), mkParam("value", Type::BOOL_TYPE) } },
        { "find_last_of_bool",  Type::INT_TYPE, { MkArrayParam("arr", Type::BOOL_TYPE), mkParam("value", Type::BOOL_TYPE) } }
    };

    return lib;
}

const std::vector<BuiltinLibrary>& BuiltinLibraryRegistry::GetAll()
{
    static std::vector<BuiltinLibrary> libs = { BuildStringLibrary(), BuildRuntimeLibrary(), BuildArrayLibrary(), BuildMathLibrary() };

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
    return std::filesystem::absolute(resolved).string();
}
