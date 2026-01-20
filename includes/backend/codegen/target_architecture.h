
#pragma once

enum class BitArchitecture 
{
    X86,      // 32-bit x86
    X86_64,   // 64-bit x86
    ARM32,    // 32-bit ARM
    ARM64,     // 64-bit ARM (AArch64)
    Unknown
};

enum class OS 
{
    Windows,
    Linux,
    Unknown
};

/*
    *  Provides target-specific information (OS + Architecture)
*/
class TargetArchitecture 
{
public:
    TargetArchitecture();
    
    // ====== Detection =======
    static TargetArch GetHostArch();
    static TargetOS GetHostOS();
    
    // ===== GETTERS =====
    TargetArch GetArch() const noexcept;
    TargetOS GetOS() const noexcept;
    
    // ===== SIZE INFO =====
    bool Is32Bit() const noexcept;
    bool Is64Bit() const noexcept;

private:
    BitArchitecture m_bitArchitecture;
    OS m_os;
};
