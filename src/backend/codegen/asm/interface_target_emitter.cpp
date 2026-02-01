#include "includes/backend/codegen/asm/interface_target_emitter.h"

std::string ITargetEmitter::GetFileExtension() const
{
    switch (TargetArchitecture::GetArch())
    {
    case BitArchitecture::X86:
    case BitArchitecture::X86_64:
        return ".asm";

    case BitArchitecture::ARM32:
    case BitArchitecture::ARM64:
        return ".s";
    
    default:
        return "";
    }
}