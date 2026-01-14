#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "includes/logger/entity_logger.h"

#include "var_table_node.h"

constexpr const int ALIGNMENT = 8;

/// @brief Holds metadata of a function.
/// @struct of a function:
///      signature + body + flags + frame size
class FuncMetaData
{
public:
    FuncMetaData(
        const EntityLogger& entityLogger,
        const std::string& funcName,
        Type funcRtype,
        const std::vector<VarMetaData>& funcParams,
        size_t declarationLine,
        size_t declarationColumn,
        std::unique_ptr<VarTableNode> functionScopeRoot = nullptr
    );
    ~FuncMetaData() = default;

    // remove copying
    FuncMetaData(const FuncMetaData&) = delete;
    FuncMetaData& operator=(const FuncMetaData&) = delete;

    // enable moving instead of copying
    FuncMetaData(FuncMetaData&&) noexcept = default;
    FuncMetaData& operator=(FuncMetaData&&) noexcept = default;

    // Getters
    const std::string& GetName() const noexcept;
    const Type& GetRType() const noexcept;
    const std::vector<VarMetaData>& GetFuncParams() const noexcept;
    bool IsFuncDefined() const noexcept;
    bool IsFuncDeclared() const noexcept;
    size_t GetDeclarationLine() const noexcept;
    size_t GetDeclarationColumn() const noexcept;

    void SetDefined() noexcept;

    /// @brief Check for given function's signature equality.
    /// @param other: Function to compare to the original.
    /// @return bool: true -> Both signatures are same, fasle -> Otherwise.
    bool HasSameSignature(const FuncMetaData& other) const;

    /// @brief Calculate total bytes of the function frame
    ///     and then sets result in `m_stackFrameSize`.
    /// @return int: result of calculation.
    int ComputeFrameSize();

private:
    const EntityLogger& m_entityLogger;

    /**********************\
    |  FUNCTION SIGNATURE  |
    \**********************/
    std::string m_name;
    Type m_rtype;
    std::vector<VarMetaData> m_params;

    /************************\
    |  DECLARATION LOCATION  |
    \************************/
    size_t m_declarationLine;
    size_t m_declarationColumn;

    /*****************\
    |  FUNCTION BODY  |
    \*****************/
    std::unique_ptr<VarTableNode> m_functionScopeRoot;

    /******************\
    |  FUNCTION FLAGS  |
    \******************/
    bool m_isDefined = false;
    bool m_isDeclared = false;

    /*****************\
    |  FUNCTION SIZE  |
    \*****************/
    int m_stackFrameSize = 0;

    /// @brief Finds the most negative (deepest) stack offset in a function's scope tree.
    /// @param node Root scope node of the function body.
    /// @return The smallest (most negative) offset found.
    static int FindFurthestOffset(const VarTableNode* node);
};
