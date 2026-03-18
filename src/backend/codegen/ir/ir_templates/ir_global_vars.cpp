#include "includes/backend/codegen/ir/ir_templates/ir_global_vars.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_string.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_int.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_float.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_bool.h"
#include "includes/backend/codegen/ir/ir_templates/ir_type.h"

IRGlobalVariable::IRGlobalVariable(
    const std::string& name, 
    std::shared_ptr<const IRType> type,
    bool isExtern
) : 
    IRBaseValue(name, type), 
    m_parent(nullptr), 
    m_initializer(nullptr),
    m_isExtern(isExtern)
{}

IRModule* IRGlobalVariable::GetParent() const noexcept
{
    return this->m_parent;
}

IRBaseValue* IRGlobalVariable::GetInitializer() const
{
    return this->m_initializer;
}

bool IRGlobalVariable::IsExtern() const
{
    return this->m_isExtern;
}

bool IRGlobalVariable::HasArrayInitializer() const noexcept
{
    return !this->m_arrayInitializer.empty();
}

const std::vector<IRBaseValue*>& IRGlobalVariable::GetArrayInitializer() const noexcept
{
    return this->m_arrayInitializer;
}

void IRGlobalVariable::SetParent(IRModule* module) noexcept
{
    this->m_parent = module;
}

void IRGlobalVariable::SetInitializer(IRBaseValue* init)
{
    this->m_initializer = init;
}

void IRGlobalVariable::SetArrayInitializer(std::vector<IRBaseValue*> elements)
{
    this->m_arrayInitializer = std::move(elements);
}

void IRGlobalVariable::SetIsExtern(bool isExtern)
{

}

std::string IRGlobalVariable::ToString() const
{
    return "@" + this->GetName();  // globals use @ prefix
}

std::string IRGlobalVariable::GetInitializerString() const
{
    if (auto* strConst = dynamic_cast<IRConstantString*>(this->m_initializer))
    {
        return strConst->GetLabel();
    }

    std::shared_ptr<const IRType> varType = this->m_initializer->GetType();
    if (varType == IRType::GetInt32() ||
        varType == IRType::GetInt64())
    {
        return std::to_string(static_cast<IRConstantInt*>(this->m_initializer)->GetValue());
    }
    else if (varType == IRType::GetFloat32() ||
        varType == IRType::GetFloat64())
    {
        return std::to_string(static_cast<IRConstantFloat*>(this->m_initializer)->GetValue());
    }
    else if (varType == IRType::GetInt1())
    {
        return static_cast<IRConstantBool*>(this->m_initializer)->GetValue() ? "1" : "0";
    }
    else
    {
        return "0";
    }
}

std::string IRGlobalVariable::GetArrayInitializerString() const
{
    std::string result;
    for (size_t i = 0; i < m_arrayInitializer.size(); ++i)
    {
        if (i > 0) result += ", ";
        IRBaseValue* elem = m_arrayInitializer[i];
        if (!elem) { result += "0"; continue; }
        auto* intConst = dynamic_cast<IRConstantInt*>(elem);
        auto* floatConst = dynamic_cast<IRConstantFloat*>(elem);
        auto* boolConst = dynamic_cast<IRConstantBool*>(elem);
        if (intConst)
            result += std::to_string(intConst->GetValue());
        else if (floatConst)
            result += std::to_string(floatConst->GetValue());
        else if (boolConst)
            result += boolConst->GetValue() ? "1" : "0";
        else
            result += "0";
    }
    return result;
}
