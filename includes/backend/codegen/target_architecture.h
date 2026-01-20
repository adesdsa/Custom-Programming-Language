#pragma once

enum class BitArchitecture 
{
    X86,      // 32-bit x86
    X86_64,   // 64-bit x86
    ARM32,    // 32-bit ARM
    ARM64,    // 64-bit ARM (AArch64)
    Unknown
};

enum class OS 
{
    Windows,
    Linux,
    Unknown
};

/**
    * @brief Provides target specific information (OS + Architecture)
    * @note class is pure static API class, no instances needed
    * @see API'S section below for available API'S
*/
class TargetArchitecture 
{
public:
    TargetArchitecture() = delete;

    // ========== Compile-Time Host Detection ==========
    static constexpr BitArchitecture HOST_ARCH =
        #if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
            BitArchitecture::X86_64

        #elif defined(_M_IX86) || defined(__i386__)
            BitArchitecture::X86

        #elif defined(_M_ARM64) || defined(__aarch64__)
            BitArchitecture::ARM64

        #elif defined(_M_ARM) || defined(__arm__)
            BitArchitecture::ARM32

        #else
            BitArchitecture::Unknown

        #endif
    ;

    static constexpr OS HOST_OS =
        #if defined(_WIN32) || defined(_WIN64)
            OS::Windows

        #elif defined(__linux__)
            OS::Linux

        #else
            OS::Unknown

        #endif
    ;

    // ====================================
    // =========== API'S ==================
    // ====================================

    static constexpr BitArchitecture GetArch() noexcept 
    { 
        return HOST_ARCH; 
    }
    static constexpr OS GetOS() noexcept 
    { 
        return HOST_OS; 
    }
    
    static constexpr bool Is32Bit() noexcept 
    { 
        return (
                HOST_ARCH == BitArchitecture::X86 || 
                HOST_ARCH == BitArchitecture::ARM32
            );
    }
    
    static constexpr bool Is64Bit() noexcept 
    { 
        return (
                HOST_ARCH == BitArchitecture::X86_64 || 
                HOST_ARCH == BitArchitecture::ARM64 
            );
    }
};
