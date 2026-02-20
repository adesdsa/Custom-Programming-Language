#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "includes/backend/codegen/ir/ir_templates/ir_module.h"
#include "includes/backend/codegen/asm/interface_target_emitter.h"
#include "includes/shared/logger/entity_logger.h"
#include "includes/shared/errors/error_manager.h"
#include "includes/shared/common/target_architecture.h"
#include "includes/frontend/semantic_analysis/func_table.h"

/**
    * @brief Converts IR into target-specific assembly and writes a .asm file.
    *
    * Selects the correct emitter based on the host architecture,
    * emits assembly from the IR module, and writes the result to disk.
*/
class ASMGenerator
{
public:
    ASMGenerator(const IRModule& program, const FuncTable& funcTable,
                 const std::string& outputName, const std::string& outputDir);
    ~ASMGenerator() = default;

    /*****************************************************************************
     * @brief Runs the ASM generation pipeline:                                  |
     *                                                                           |
     * 1. Select target emitter based on host architecture (x86 / x86_64)        |
     * 2. Emit assembly string from the IR module                                |
     * 3. Write the assembly to an output .asm file                              |
     *                                                                           |
     * @return std::string  Path to the generated .asm file                      |
     ****************************************************************************/
    std::string MainFlow();

private:
    const IRModule& m_irMoudle;
    const FuncTable& m_funcTable;
    std::string m_outputName;
    std::string m_outputDir;

    EntityLogger m_ASMLogger;
    ErrorManager& m_errorManager;

    /// @brief Creates the appropriate emitter for the host architecture
    std::unique_ptr<ITargetEmitter> CreateEmitter() const;

    /// @brief Writes assembly content to a file and returns the output path
    std::string WriteAsmToFile(const std::string& asmContent, const std::string& fileExtension) const;
};
