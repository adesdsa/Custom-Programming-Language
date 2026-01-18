
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "includes/backend/codegen/ir/ir_function.h"
#include "includes/backend/codegen/ir/ir_global_vars.h"

/// @brief Represents an entire IR module, containing functions and global variables.
///
/// IRModule owns all its functions and global variables, storing them in vectors for
/// ownership and maps for fast lookup by name.
///
/// - Functions are stored in `m_functions` (vector) and `m_functionMap` (map).  
/// - Global variables are stored in `m_globals` (vector) and `m_globalMap` (map).  
/// - Provides getters, adders, and existence checks for both functions and globals.  
/// - Names of functions and globals are expected to be unique within the module.
class IRModule 
{
public:
    IRModule(const std::string& moduleName);

    const std::string& GetName() const noexcept;

    // <======= IRFunction Functions =======>
    IRFunction* GetFunction(const std::string& name) const;
    const std::vector<std::unique_ptr<IRFunction>>& GetFunctions() const noexcept;
    /// @return m_functions size
    size_t GetNumFunctions() const noexcept;
    /// @return nullptr -> Func with this name already exists. otherwise -> success.
    IRFunction* AddFunction(const std::string& name, std::shared_ptr<const IRType> returnType);
    bool IsFuncExistByName(const std::string& name) const;

    // <======= IRGlobalVariable Functions =======>
    IRGlobalVariable* GetVariable(const std::string& name) const;
    const std::vector<std::unique_ptr<IRGlobalVariable>>& GetVariables() const noexcept;
    /// @return m_globals size
    size_t GetNumVariables() const noexcept;
    /// @return nullptr -> Var with this name already exists. otherwise -> success.
    IRGlobalVariable* AddVariable(const std::string& name, std::shared_ptr<const IRType> type);
    bool IsVarExistByName(const std::string& name) const;

private:
    const std::string m_name;
    
    /// @note: Following code contains both vector and map for better time complexity
                            
    std::vector<std::unique_ptr<IRFunction>> m_functions;
    std::map<std::string, IRFunction*> m_functionMap;

    std::vector<std::unique_ptr<IRGlobalVariable>> m_globals;
    std::map<std::string, IRGlobalVariable*> m_globalMap;
};
