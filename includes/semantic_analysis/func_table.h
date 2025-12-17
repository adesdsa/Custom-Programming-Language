#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "var_table_node.h"

struct FuncMetaData
{
    /**********************\
    |  FUNCTION SIGNATURE  |
    \**********************/
    std::string name;
    Type rtype;

    std::vector<VarMetaData> params;


    /*****************\
    |  FUNCTION BODY  |
    \*****************/
    
    // the outter block of the function. (parent = global scope)
    std::unique_ptr<VarTableNode> functionScopeRoot;


    /******************\
    |  FUNCTION FLAGS  |
    \******************/
    bool isDeclared = false;
    bool isDefined  = false;


    /*****************\
    |  FUNCTION SIZE  |
    \*****************/
    int stackFrameSize = 0;
};

// Holds all functions (signature + body) of certain file
class FuncTable
{
public:
    FuncTable() = default;
    ~FuncTable() = default;
    
    // @brief: Search through the table for given function and return pointer to it.
    // @param funcName: Function's name to search for.
    // @return: Pointer to the function's metadata -> if found, nullptr -> Otherwise.
    FuncMetaData* FindFunctionByName(const std::string& funcName);

    // @brief: Inserts a function if it doesn't exist yet.
    // @param funcMetadata: metadata of the function we want to insert.
    // @return: true -> Inserting successfully, false -> Function already exist.
    // @note: && means passing ownership of this object.
    bool InsertFunction(FuncMetaData&& funcMetadata);

    // @brief: Check for given function's signature equality.
    // param func1: First function to compare.
    // param func2: Second function to compare.
    // return: true -> Both signatures are same, fasle -> Otherwise.
    static bool CompareFunctionsSignature(
        const FuncMetaData& func1, 
        const FuncMetaData func2
    );

private:
    // Key: function name, Value: FuncMetaData struct
    std::unordered_map<std::string, std::unique_ptr<FuncMetaData>> m_functions;
};
