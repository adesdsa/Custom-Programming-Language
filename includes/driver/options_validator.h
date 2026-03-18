#pragma once

#include "includes/driver/compiler_options.h" // For CompilerOptions struct

/// @brief Validates a fully-populated CompilerOptions struct.
///
/// This class is UI-agnostic: any frontend (CLI, GUI, web, etc.)
/// should call Validate() after building its CompilerOptions
/// to enforce common rules (required fields, mutual exclusivity, etc.).
class OptionsValidator
{
public:
    /// @brief Validate the given options.
    /// @param options the populated CompilerOptions to check
    /// @throws std::runtime_error if any rule is violated
    static void Validate(const CompilerOptions& options);

private:
    OptionsValidator() = delete; // static-only class
};
