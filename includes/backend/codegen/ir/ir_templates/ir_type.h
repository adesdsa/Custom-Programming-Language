#pragma once

#include <memory>
#include <vector>

/// @note: all types based on bits. e.g., Int1 = 1 bit
enum class IRTypeKind 
{
    Void,
    Int1,
    Int8,
    Int32, 
    Int64, 
    Float32, 
    Float64,
    Function  // non fixed amount of bits
};

/// @brief A class that represents the special intermidiate language 
/// types (please @see IRTypeKind enumerator).
class IRType 
{
public:
    /// @note The second and third params are only used when creating a func.
    IRType(const IRTypeKind k, 
        std::shared_ptr<const IRType> funcReturnType = nullptr, 
        std::vector<std::shared_ptr<const IRType>> funcParams = {});
    ~IRType() = default;

    // ===== STATIC FACTORY METHODS =====
    static std::shared_ptr<const IRType> GetVoid();

    static std::shared_ptr<const IRType> GetInt1();

    static std::shared_ptr<const IRType> GetInt8();

    static std::shared_ptr<const IRType> GetInt32();

    static std::shared_ptr<const IRType> GetInt64();

    static std::shared_ptr<const IRType> GetFloat32();

    static std::shared_ptr<const IRType> GetFloat64();

    static std::shared_ptr<const IRType> GetFunction(
        std::shared_ptr<const IRType> returnType, 
        std::vector<std::shared_ptr<const IRType>> paramTypes
    );

    // ===== INSTANCE METHODS =====
    size_t GetSizeInBytes() const noexcept;

    IRTypeKind GetKind() const noexcept;
    std::shared_ptr<const IRType> GetReturnType() const noexcept;
    const std::vector<std::shared_ptr<const IRType>>& GetParameters() const noexcept;

private:
    const IRTypeKind m_kind;
    
    // in case of a function
    std::shared_ptr<const IRType> m_returnType;
    std::vector<std::shared_ptr<const IRType>> m_paramTypes;
};
