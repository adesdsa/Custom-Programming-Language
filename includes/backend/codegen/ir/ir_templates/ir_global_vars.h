#pragma once

#include <string>
#include <memory>
#include <vector>

#include "includes/backend/codegen/ir/ir_templates/ir_base_value.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_bool.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_int.h"
#include "includes/backend/codegen/ir/ir_templates/ir_constant_values/ir_constant_float.h"

class IRModule;

/// @brief Represents a global variable in an IR module.
///
/// IRGlobalVariable inherits from IRBaseValue and tracks its parent module and optional initializer.
///
/// - `m_parent` points to the IRModule that owns this variable.  
/// - `m_initializer` optionally points to an IRBaseValue representing the variable's initial value.  
/// - `m_arrayInitializer` holds element values for array initializers (e.g. [1,2,3]).
class IRGlobalVariable : public IRBaseValue 
{
public:
    IRGlobalVariable(const std::string& name, std::shared_ptr<const IRType> type, bool isExtern = false);

    // <======= Getters =======>
    IRModule* GetParent() const noexcept;
    IRBaseValue* GetInitializer() const;
    bool IsExtern() const;
    bool HasArrayInitializer() const noexcept;
    const std::vector<IRBaseValue*>& GetArrayInitializer() const noexcept;

    // <======= Setters =======>
    void SetParent(IRModule* module) noexcept;
    void SetInitializer(IRBaseValue* init);
    void SetIsExtern(bool isExtern);
    void SetArrayInitializer(std::vector<IRBaseValue*> elements);

    std::string ToString() const override;

    std::string GetInitializerString() const;
    /// @brief For array initializers, returns comma-separated values for .data section.
    std::string GetArrayInitializerString() const;

private:
    IRModule* m_parent;
    IRBaseValue* m_initializer;  // initial value (optional)
    bool m_isExtern; // Extern globals should NOT be emitted in .data/.bss sections
    std::vector<IRBaseValue*> m_arrayInitializer;  // for array initializers [a,b,c,...]
};
