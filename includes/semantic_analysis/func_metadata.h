#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "var_table_node.h"

constexpr const int ALIGNMENT = 8;

class FuncMetaData
{
public:
    FuncMetaData() = default;
    ~FuncMetaData() = default;

    /// @brief Check for given function's signature equality.
    /// @param other: Function to compare to the original.
    /// @return bool: true -> Both signatures are same, fasle -> Otherwise.
    bool HasSameSignature(const FuncMetaData& other) const;

    /// @brief Calculate total bytes of the function frame
    ///     and then sets result in `m_stackFrameSize`.
    /// @return int: result of calculation.
    int ComputeFrameSize();

private:
    /**********************\
    |  FUNCTION SIGNATURE  |
    \**********************/
    std::string m_name;
    Type m_rtype;
    std::vector<VarMetaData> m_params;

    /*****************\
    |  FUNCTION BODY  |
    \*****************/
    std::unique_ptr<VarTableNode> m_functionScopeRoot;

    /******************\
    |  FUNCTION FLAGS  |
    \******************/
    bool m_isDeclared = false;
    bool m_isDefined = false;

    /*****************\
    |  FUNCTION SIZE  |
    \*****************/
    int m_stackFrameSize = 0;

    /// @brief Finds the most negative (deepest) stack offset in a function's scope tree.
    /// @param node Root scope node of the function body.
    /// @return The smallest (most negative) offset found.
    static int FindFurthestOffset(const VarTableNode* node);
};
