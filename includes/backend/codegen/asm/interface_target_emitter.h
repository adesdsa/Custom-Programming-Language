#pragma once

#include <string>

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/target_architecture.h"

// Emitter Interface, ASM generation will be through those
// API calls
class ITargetEmitter
{
public:
    virtual ~ITargetEmitter() = default;

    /// Convert given IRMoudle to a full ASM file
    /// @param irProgram full IR program to convert to ASM
    /// @returns `std::string` ASM code to write to output file
    virtual std::string Emit(const IRModule& irProgram) = 0;

    /// Get file extenstion based on bit architecture
    virtual std::string GetFileExtension() const;
};
