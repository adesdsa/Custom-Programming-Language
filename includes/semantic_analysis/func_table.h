#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "func_metadata.h"

/// @brief Holds all functions (signature + body) of certain file
class FuncTable
{
public:
    FuncTable() = default;
    ~FuncTable() = default;
    
    /// @brief: Search through the table for given function and return pointer to it.
    /// @param funcName: Function's name to search for.
    /// @return: Pointer to the function's metadata -> if found, nullptr -> Otherwise.
    FuncMetaData* FindFunctionByName(const std::string& funcName) const;

    /// @brief Inserts a function if it doesn't exist yet.
    /// @param funcMetadata metadata of the function we want to insert.
    /// @return bool: true -> Inserting successfully, false -> Function already exist.
    /// @note && means passing ownership of this object.
    bool InsertFunction(FuncMetaData&& funcMetadata);

private:
    // Key: function name, Value: FuncMetaData struct
    std::unordered_map<std::string, std::unique_ptr<FuncMetaData>> m_functions;
};
