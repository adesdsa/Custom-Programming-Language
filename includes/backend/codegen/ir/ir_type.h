#pragma once

#include <memory>
#include <vector>

#include "includes/semantic_analysis/func_metadata.h"

/// @note: all types based on bits. e.g., Int1 = 1 bit
enum class IRTypeKind {
    Void,
    Int1,
    Int8,
    Int32, 
    Int64, 
    Float32, 
    Float64,
    Function  // non fixed amount of bits
};

class IRType 
{
public:
    IRType(IRTypeKind k);
    ~IRType() = default;

    // ===== STATIC FACTORY METHODS =====
    static std::shared_ptr<IRType> GetVoid() noexcept;

    static std::shared_ptr<IRType> GetInt1() noexcept;

    static std::shared_ptr<IRType> GetInt8() noexcept;

    static std::shared_ptr<IRType> GetInt32() noexcept;

    static std::shared_ptr<IRType> GetInt64() noexcept;

    static std::shared_ptr<IRType> GetFloat32() noexcept;

    static std::shared_ptr<IRType> GetFloat64() noexcept;

    static std::shared_ptr<IRType> GetFunction(
        std::shared_ptr<IRType> returnType, 
        std::vector<std::shared_ptr<IRType>> paramTypes
    ) noexcept;

    // ===== INSTANCE METHODS =====
    size_t GetSizeInBytes() const noexcept;

    std::shared_ptr<IRType> GetReturnType() const noexcept;
    std::vector<std::shared_ptr<IRType>> GetParameters() const noexcept;

private:
    IRTypeKind m_kind;
    
    // in case of a function
    std::shared_ptr<IRType> m_returnType;
    std::vector<std::shared_ptr<IRType>> m_paramTypes;
};

