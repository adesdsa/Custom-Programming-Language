#include "includes/backend/codegen/target_architecture.h"

TargetArchitecture::TargetArchitecture()
{
    this->m_bitArchitecture = this->GetHostArch();
    this->m_os = this->GetHostOS();
}

TargetArch TargetArchitecture::GetHostArch()
{
    // ========= 32-bit Systems ===========
    #if defined(_M_IX86) || defined(__i386__)
        // 32-bit x86
        #define TARGET_ARCH BitArchitecture::X86
        #define TARGET_32BIT 1

    #elif defined(_M_ARM) || defined(__arm__)
        // 32-bit ARM
        #define TARGET_ARCH BitArchitecture::ARM32
        #define TARGET_32BIT 1

    // ========= 64-bit Systems ===========
    #elif defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
        // 64-bit x86 (Intel/AMD)
        #define TARGET_ARCH BitArchitecture::X86_64
        #define TARGET_64BIT 1

    #elif defined(_M_ARM64) || defined(__aarch64__)
        // 64-bit ARM
        #define TARGET_ARCH BitArchitecture::ARM64
        #define TARGET_64BIT 1

    #else
        #define TARGET_ARCH BitArchitecture::Unknown

    #endif
}

TargetOS TargetArchitecture::GetHostOS()
{
    #if defined(_WIN32) || defined(_WIN64)
        #define TARGET_OS OS::Windows

    #elif defined(__linux__)
        #define TARGET_OS OS::Linux

    #else
        #define TARGET_OS OS::Unknown

    #endif
}

TargetArch TargetArchitecture::GetArch() const noexcept
{
    return this->m_bitArchitecture;
}

TargetOS TargetArchitecture::GetOS() const noexcept
{
    return this->m_os;
}

bool TargetArchitecture::Is32Bit() const noexcept
{
    switch (this->m_bitArchitecture)
    {
        case BitArchitecture::X86:
            return true;

        case BitArchitecture::ARM32:
            return true;
    }

    return false;
}

bool TargetArchitecture::Is64Bit() const noexcept
{
    switch (this->m_bitArchitecture)
    {
        case BitArchitecture::X86_64:
            return true;

        case BitArchitecture::ARM64:
            return true;
    }

    return false;
}
