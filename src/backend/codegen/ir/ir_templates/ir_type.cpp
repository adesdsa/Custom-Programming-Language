#include "includes/backend/codegen/ir/ir_templates/ir_type.h"

IRType::IRType(
    const IRTypeKind kind, 
    std::shared_ptr<const IRType> funcReturnType, 
    std::vector<std::shared_ptr<const IRType>> funcParams
) : 
    m_kind(kind),
    m_returnType(funcReturnType),
    m_paramTypes(funcParams),
    m_elementType(nullptr),
    m_arrayCount(0)
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
    auto instance = std::make_shared<const IRType>(
        IRTypeKind::Function,
        std::move(returnType),
        std::move(paramTypes)
    );
    return instance;
}

IRType::IRType(std::shared_ptr<const IRType> elementType, size_t arrayCount)
    : m_kind(IRTypeKind::Array),
      m_returnType(nullptr),
      m_paramTypes(),
      m_elementType(std::move(elementType)),
      m_arrayCount(arrayCount)
{}

std::shared_ptr<const IRType> IRType::GetArray(
    std::shared_ptr<const IRType> elementType,
    size_t count)
{
    // Use new (not make_shared) so the private Array constructor is accessible from within GetArray
    return std::shared_ptr<const IRType>(new IRType(std::move(elementType), count));
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
        case IRTypeKind::Array:
            return m_elementType ? m_elementType->GetSizeInBytes() * m_arrayCount : 0;
        default: return 0;
    }
}

IRTypeKind IRType::GetKind() const noexcept
{
    return this->m_kind;
}

std::shared_ptr<const IRType> IRType::GetReturnType() const noexcept
{
    return this->m_returnType;
}

const std::vector<std::shared_ptr<const IRType>>& IRType::GetParameters() const noexcept
{
    return this->m_paramTypes;
}

std::shared_ptr<const IRType> IRType::GetElementType() const noexcept
{
    return this->m_elementType;
}

size_t IRType::GetArrayCount() const noexcept
{
    return this->m_arrayCount;
}
