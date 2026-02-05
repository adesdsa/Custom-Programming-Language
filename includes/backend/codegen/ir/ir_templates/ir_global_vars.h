#pragma once

#include <string>
#include <memory>

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
class IRGlobalVariable : public IRBaseValue 
{
public:
    IRGlobalVariable(const std::string& name, std::shared_ptr<const IRType> type);

    // <======= Getters =======>
    IRModule* GetParent() const noexcept;
    IRBaseValue* GetInitializer() const;

    // <======= Setters =======>
    void SetParent(IRModule* module) noexcept;
    void SetInitializer(IRBaseValue* init);

    std::string ToString() const override;

    std::string GetInitializerString() const;

private:
    IRModule* m_parent;
    IRBaseValue* m_initializer;  // initial value (optional)
};
