#include "includes/frontend/semantic_analysis/func_table.h"

FuncMetaData* FuncTable::FindFunctionByName(const std::string& funcName) const
{
    auto function = this->m_functions.find(funcName);
    return function != this->m_functions.end() ? function->second.get() : nullptr;
}

bool FuncTable::InsertFunction(FuncMetaData&& funcMetadata)
{
    auto existingFunction = this->FindFunctionByName(funcMetadata.GetName());
    // We use move and only then GetName, by this point the metada is empty so we need to save the name.
    std::string sFuncName = funcMetadata.GetName();
    
    if (existingFunction)
    {
        if (
            existingFunction->IsFuncDeclared() && 
            !existingFunction->IsFuncDefined() && 
            funcMetadata.IsFuncDefined()
        )
        {
            this->m_entityLogger.Info("Insert function declaration to an existing function definition");
            this->m_functions[sFuncName] =
                std::make_unique<FuncMetaData>(std::move(funcMetadata));
            return true;
        }
        this->m_entityLogger.Warn(
            "Function " + sFuncName + " already exists. Inseration failed"
        );
        return false;
    }

    this->m_entityLogger.Info("Inserting a new function into function table");
    this->m_functions[sFuncName] = std::make_unique<FuncMetaData>(
        std::move(funcMetadata)
    );
    return true;
}

const std::unordered_map<std::string, std::unique_ptr<FuncMetaData>>&
FuncTable::GetAllFunctions() const noexcept
{
    return this->m_functions;
}

void FuncTable::RegisterBuiltins()
{
    // guard: only register once
    if (this->FindFunctionByName("println")) return;

    // println(value: int) -> void
    std::vector<VarMetaData> printlnParams = {
        VarMetaData("value", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData printlnFunc(
        this->m_entityLogger,
        "println",
        Type::VOID_TYPE,
        printlnParams,
        0, 0,  // no source location (built-in)
        nullptr // no scope root (built-in)
    );
    printlnFunc.SetDefined(); // mark as defined so the linker check passes
    this->InsertFunction(std::move(printlnFunc));

    // str_concat(a: string, b: string) -> string
    std::vector<VarMetaData> strConcatParams = {
        VarMetaData("a", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("b", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strConcatFunc(
        this->m_entityLogger,
        "str_concat",
        Type::STRING_TYPE,
        strConcatParams,
        0, 0,
        nullptr
    );
    strConcatFunc.SetDefined();
    this->InsertFunction(std::move(strConcatFunc));

    // str_from_char(c: char) -> string
    std::vector<VarMetaData> strFromCharParams = {
        VarMetaData("c", Type::CHAR_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strFromCharFunc(
        this->m_entityLogger,
        "str_from_char",
        Type::STRING_TYPE,
        strFromCharParams,
        0, 0,
        nullptr
    );
    strFromCharFunc.SetDefined();
    this->InsertFunction(std::move(strFromCharFunc));

    // str_cmp(a: string, b: string) -> int
    std::vector<VarMetaData> strCmpParams = {
        VarMetaData("a", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("b", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strCmpFunc(
        this->m_entityLogger,
        "str_cmp",
        Type::INT_TYPE,
        strCmpParams,
        0, 0,
        nullptr
    );
    strCmpFunc.SetDefined();
    this->InsertFunction(std::move(strCmpFunc));

    // str_len(s: string) -> int
    std::vector<VarMetaData> strLenParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strLenFunc(
        this->m_entityLogger,
        "str_len",
        Type::INT_TYPE,
        strLenParams,
        0, 0,
        nullptr
    );
    strLenFunc.SetDefined();
    this->InsertFunction(std::move(strLenFunc));

    // substr(s: string, start: int, length: int) -> string
    std::vector<VarMetaData> substrParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("start", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("length", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData substrFunc(
        this->m_entityLogger,
        "substr",
        Type::STRING_TYPE,
        substrParams,
        0, 0,
        nullptr
    );
    substrFunc.SetDefined();
    this->InsertFunction(std::move(substrFunc));

    // substr_step(s: string, start: int, end: int, step: int) -> string
    std::vector<VarMetaData> substrStepParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("start", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("end", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("step", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData substrStepFunc(
        this->m_entityLogger,
        "substr_step",
        Type::STRING_TYPE,
        substrStepParams,
        0, 0,
        nullptr
    );
    substrStepFunc.SetDefined();
    this->InsertFunction(std::move(substrStepFunc));

    // char_at(s: string, index: int) -> char
    std::vector<VarMetaData> charAtParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("index", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData charAtFunc(
        this->m_entityLogger,
        "char_at",
        Type::CHAR_TYPE,
        charAtParams,
        0, 0,
        nullptr
    );
    charAtFunc.SetDefined();
    this->InsertFunction(std::move(charAtFunc));

    // set_char_at(s: string, index: int, c: char) -> void
    std::vector<VarMetaData> setCharAtParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("index", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("c", Type::CHAR_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData setCharAtFunc(
        this->m_entityLogger,
        "set_char_at",
        Type::VOID_TYPE,
        setCharAtParams,
        0, 0,
        nullptr
    );
    setCharAtFunc.SetDefined();
    this->InsertFunction(std::move(setCharAtFunc));

    // str_slice_set(s: string, start: int, end_idx: int, replacement: string) -> void
    std::vector<VarMetaData> strSliceSetParams = {
        VarMetaData("s", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("start", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("end_idx", Type::INT_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0),
        VarMetaData("replacement", Type::STRING_TYPE, AccessRights::DEFAULT_ACCESS, Scope::LET, 0)
    };
    FuncMetaData strSliceSetFunc(
        this->m_entityLogger,
        "str_slice_set",
        Type::VOID_TYPE,
        strSliceSetParams,
        0, 0,
        nullptr
    );
    strSliceSetFunc.SetDefined();
    this->InsertFunction(std::move(strSliceSetFunc));

    // readln() -> string
    FuncMetaData readlnFunc(
        this->m_entityLogger,
        "readln",
        Type::STRING_TYPE,
        std::vector<VarMetaData>{},
        0, 0,
        nullptr
    );
    readlnFunc.SetDefined();
    this->InsertFunction(std::move(readlnFunc));

    // read_int() -> int
    FuncMetaData read_intFunc(
        this->m_entityLogger,
        "read_int",
        Type::INT_TYPE,
        std::vector<VarMetaData>{},
        0, 0,
        nullptr
    );
    read_intFunc.SetDefined();
    this->InsertFunction(std::move(read_intFunc));

    // read_double() -> double
    FuncMetaData read_doubleFunc(
        this->m_entityLogger,
        "read_double",
        Type::DOUBLE_TYPE,
        std::vector<VarMetaData>{},
        0, 0,
        nullptr
    );
    read_doubleFunc.SetDefined();
    this->InsertFunction(std::move(read_doubleFunc));

    this->m_entityLogger.Info("Registered built-in functions");
}