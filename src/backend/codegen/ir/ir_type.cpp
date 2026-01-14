#include "includes/backend/codegen/ir/ir_type.h"

IRType::IRType(IRTypeKind k) : m_kind(k),  m_returnType(nullptr) {}

std::shared_ptr<IRType> IRType::GetVoid() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Void);
    return instance;
}

std::shared_ptr<IRType> IRType::GetInt1() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Int1);
    return instance;
}

std::shared_ptr<IRType> IRType::GetInt8() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Int8);
    return instance;
}

std::shared_ptr<IRType> IRType::GetInt32() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Int32);
    return instance;
}

std::shared_ptr<IRType> IRType::GetInt64() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Int64);
    return instance;
}

std::shared_ptr<IRType> IRType::GetFloat32() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Float32);
    return instance;
}

std::shared_ptr<IRType> IRType::GetFloat64() noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Float64);
    return instance;
}

std::shared_ptr<IRType> IRType::GetFunction(
    std::shared_ptr<IRType> returnType, 
    std::vector<std::shared_ptr<IRType>> paramTypes) noexcept
{
    static auto instance = std::make_shared<IRType>(IRTypeKind::Function);
    instance->m_returnType = returnType;
    instance->m_paramTypes = paramTypes;
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

std::shared_ptr<IRType> IRType::GetReturnType() const noexcept
{
    return this->m_returnType;
}

std::vector<std::shared_ptr<IRType>> IRType::GetParameters() const noexcept
{
    return this->m_paramTypes;
}
