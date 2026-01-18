#include "includes/backend/codegen/ir/ir_type.h"

IRType::IRType(
    const IRTypeKind k, 
    std::shared_ptr<const IRType> funcReturnType, 
    std::vector<std::shared_ptr<const IRType>> funcParams
) : 
    m_kind(k),
    m_returnType(funcReturnType),
    m_paramTypes(funcParams)
{}

std::shared_ptr<const IRType> IRType::GetVoid() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Void);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetInt1() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Int1);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetInt8() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Int8);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetInt32() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Int32);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetInt64() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Int64);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetFloat32() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Float32);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetFloat64() 
{
    static auto instance = std::make_shared<const IRType>(IRTypeKind::Float64);
    return instance;
}

std::shared_ptr<const IRType> IRType::GetFunction(
    std::shared_ptr<const IRType> returnType, 
    std::vector<std::shared_ptr<const IRType>> paramTypes) 
{
    auto instance = std::make_shared<const IRType>(IRTypeKind::Function,
        std::move(returnType),
        std::move(paramTypes));
    return instance;
}

size_t IRType::GetSizeInBytes() const noexcept
{
    switch (this->m_kind) 
    {
        case IRTypeKind::Void:    return 0;
        case IRTypeKind::Int1:    return 1;
        case IRTypeKind::Int8:    return 1;
        case IRTypeKind::Int32:   return 4;
        case IRTypeKind::Int64:   return 8;
        case IRTypeKind::Float32: return 4;
        case IRTypeKind::Float64: return 8;
        case IRTypeKind::Function: return 0;  // nosize
        default: return 0;
    }
}

IRTypeKind IRType::GetKind() const noexcept
{
    return m_kind;
}

std::shared_ptr<const IRType> IRType::GetReturnType() const noexcept
{
    return this->m_returnType;
}

const std::vector<std::shared_ptr<const IRType>>& IRType::GetParameters() const noexcept
{
    return this->m_paramTypes;
}
