#include "includes/backend/linker/linker.h"

#ifdef _WIN32
#include <Windows.h>
#include <fstream>
#endif

Linker::Linker() :
    m_logger("LINKER")
{
    this->m_logger.Info("Linker component initialized successfully");
}

std::string Linker::MainFlow(
    const std::vector<std::string>& objFilePaths,
    const std::string& outputName,
    const std::string& outputDir)
{
    if (objFilePaths.empty())
    {
        this->m_logger.Error("No object files provided");
        throw std::runtime_error("Linker failed: no object files provided");
    }

    for (const auto& objPath : objFilePaths)
    {
        if (!std::filesystem::exists(objPath))
        {
            this->m_logger.Error("Object file not found: " + objPath);
            throw std::runtime_error("Linker failed: object file not found: " + objPath);
        }
    }

    std::string exePath = this->DeriveOutputPath(outputName, outputDir);
    std::string command = this->BuildLinkerCommand(objFilePaths, exePath);

    this->m_logger.Info("Linking: " + command);

    std::string output;
    int exitCode = SystemUtils::RunProcess(command, output);

    if (exitCode != OK_EXIT_CODE)
    {
        this->m_logger.Error("Linker failed (exit code " + std::to_string(exitCode) + ")");

        if (!output.empty())
        {
            this->m_logger.Error("Linker output:\n" + output);
        }

        throw std::runtime_error("Linking failed with exit code " + std::to_string(exitCode));
    }

    if (!output.empty())
    {
        this->m_logger.Info("Linker output: " + output);
    }

    this->m_logger.Info("Linked successfully: " + exePath);
    return exePath;
}

std::string Linker::BuildLinkerCommand(
    const std::vector<std::string>& objPaths,
    const std::string& exePath) const
{
    #if defined(_WIN32) || defined(_WIN64)

        if constexpr (!TargetArchitecture::Is64Bit())
        {
            // Win32: use MSVC link.exe so GetStdHandle@4 / ReadFile@20 resolve from kernel32.lib
            std::string msvcCmd = BuildMsvcLinkCommandWin32(objPaths, exePath);
            if (!msvcCmd.empty())
                return msvcCmd;
            // Fallback to GoLink (may fail with undefined GetStdHandle@4)
        }

        std::string golinkPath = SystemUtils::BuildRelativePath(L"tools/golink/GoLink.exe").string();
        std::string entryPoint;

        if constexpr (TargetArchitecture::Is64Bit())
        {
            entryPoint = "mainCRTStartup";
        }
        else
        {
            entryPoint = "_mainCRTStartup";
        }

        /**
            * /console  -> produce a console application
            * /entry    -> set the entry point symbol
            * kernel32.dll -> resolve ExitProcess and other WinAPIs
            *
            * The inner command is wrapped with cmd /C "..." so that
            * _popen (which itself invokes cmd /C) does not strip the
            * leading/trailing quotes from the executable path — this
            * is required when the path contains spaces.
        */
        std::string inner =
            "\"" + golinkPath + "\""
            + " /console"
            + " /entry " + entryPoint
            + " /fo \"" + exePath + "\"";

        for (const auto& objPath : objPaths)
        {
            inner += " \"" + objPath + "\"";
        }

        /* /mix: allow mixing object formats if needed */
        inner += " /mix";

        if constexpr (TargetArchitecture::Is64Bit())
        {
            inner += " kernel32.dll msvcrt.dll";
        }
        else
        {
            /* 32-bit: use SysWOW64 kernel32 so GoLink resolves GetStdHandle@4, ReadFile@20, etc. */
            char sysRoot[MAX_PATH];
            if (GetEnvironmentVariableA("SystemRoot", sysRoot, sizeof(sysRoot)) != 0)
            {
                std::string kernel32 = std::string(sysRoot) + "\\SysWOW64\\kernel32.dll";
                std::string msvcrt = std::string(sysRoot) + "\\SysWOW64\\msvcrt.dll";
                inner += " \"" + kernel32 + "\" \"" + msvcrt + "\"";
            }
            else
            {
                inner += " kernel32.dll msvcrt.dll";
            }
        }

        return "cmd /C \"" + inner + "\"";

    #elif defined(__linux__)

        /* -dynamic-linker: embed correct PT_INTERP so the kernel finds the loader.
         * Without this, ld may produce wrong interpreter (e.g. /lib/ld64.so.1). */
        std::string cmd;

        if constexpr (TargetArchitecture::Is64Bit())
        {
            cmd = "ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o \"" + exePath + "\"";
        }
        else
        {
            cmd = "ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o \"" + exePath + "\"";
        }

        for (const auto& objPath : objPaths)
        {
            cmd += " \"" + objPath + "\"";
        }

        cmd += " -lc";
        return cmd;

    #else
        throw std::runtime_error("Unsupported OS for linking");
    #endif
}

std::string Linker::DeriveOutputPath(
    const std::string& outputName,
    const std::string& outputDir) const
{
    std::filesystem::path dir(outputDir);
    std::filesystem::path exePath;

    if constexpr (TargetArchitecture::GetOS() == OS::Windows)
    {
        std::string name = outputName;
        if (name.size() >= 4 && (name.compare(name.size() - 4, 4, ".exe") == 0))
            exePath = dir / name;
        else
            exePath = dir / (name + ".exe");
    }
    else
    {
        exePath = dir / outputName;
    }

    return std::filesystem::absolute(exePath).string();
}

#ifdef _WIN32
std::string Linker::BuildMsvcLinkCommandWin32(
    const std::vector<std::string>& objPaths,
    const std::string& exePath)
{
    namespace fs = std::filesystem;
    const char* vsRoots[] = {
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional",
    };
    std::string vsRoot;
    for (const char* root : vsRoots)
    {
        if (fs::exists(fs::path(root) / "VC" / "Auxiliary" / "Build" / "vcvars32.bat"))
        {
            vsRoot = root;
            break;
        }
    }
    if (vsRoot.empty())
        return "";

    fs::path vcToolsVersionFile = fs::path(vsRoot) / "VC" / "Auxiliary" / "Build" / "Microsoft.VCToolsVersion.default.txt";
    std::string toolVersion;
    if (fs::exists(vcToolsVersionFile))
    {
        std::ifstream f(vcToolsVersionFile);
        if (f)
        {
            std::getline(f, toolVersion);
            while (!toolVersion.empty() && (toolVersion.back() == '\r' || toolVersion.back() == ' '))
                toolVersion.pop_back();
        }
    }
    if (toolVersion.empty())
        return "";

    fs::path linkExe = fs::path(vsRoot) / "VC" / "Tools" / "MSVC" / toolVersion / "bin" / "Hostx86" / "x86" / "link.exe";
    if (!fs::exists(linkExe))
    {
        linkExe = fs::path(vsRoot) / "VC" / "Tools" / "MSVC" / toolVersion / "bin" / "Hostx64" / "x86" / "link.exe";
        if (!fs::exists(linkExe))
            return "";
    }

    fs::path vcvars32 = fs::path(vsRoot) / "VC" / "Auxiliary" / "Build" / "vcvars32.bat";
    std::string vcvars32Str = vcvars32.string();
    std::string linkExeStr = linkExe.string();
    fs::path vcLibX86 = fs::path(vsRoot) / "VC" / "Tools" / "MSVC" / toolVersion / "lib" / "x86";

    const char* sdkVersions[] = { "10.0.22621.0", "10.0.19041.0", "10.0.20348.0" };
    const char* sdkRoot = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib";
    std::string sdkUmPath;
    std::string sdkUcrtPath;
    for (const char* ver : sdkVersions)
    {
        fs::path sdkUm = fs::path(sdkRoot) / ver / "um" / "x86";
        fs::path sdkUcrt = fs::path(sdkRoot) / ver / "ucrt" / "x86";
        if (fs::exists(sdkUm))
            sdkUmPath = sdkUm.string();
        if (fs::exists(sdkUcrt))
            sdkUcrtPath = sdkUcrt.string();
        if (!sdkUmPath.empty() || !sdkUcrtPath.empty())
            break;
    }

    fs::path outDir = fs::path(exePath).parent_path();
    fs::path batPath = outDir / "picoc_link_win32.bat";
    std::ofstream bat(batPath);
    if (!bat)
        return "";
    bat << "@echo off\ncall \"" << vcvars32Str << "\"\n";
    if (!sdkUmPath.empty() || !sdkUcrtPath.empty() || fs::exists(vcLibX86))
    {
        bat << "set LIB=";
        if (fs::exists(vcLibX86))
            bat << "\"" << vcLibX86.string() << "\";";
        if (!sdkUmPath.empty())
            bat << "\"" << sdkUmPath << "\";";
        if (!sdkUcrtPath.empty())
            bat << "\"" << sdkUcrtPath << "\";";
        bat << "%LIB%\n";
    }
    bat << "\"" << linkExeStr << "\" /nologo /subsystem:console /entry:_mainCRTStartup /out:\"" << exePath << "\"";
    if (fs::exists(vcLibX86))
        bat << " /libpath:\"" << vcLibX86.string() << "\"";
    if (!sdkUmPath.empty())
        bat << " /libpath:\"" << sdkUmPath << "\"";
    if (!sdkUcrtPath.empty())
        bat << " /libpath:\"" << sdkUcrtPath << "\"";
    for (const auto& objPath : objPaths)
        bat << " \"" << objPath << "\"";
    if (!sdkUmPath.empty())
        bat << " \"" << sdkUmPath << "\\kernel32.lib\"";
    else
        bat << " kernel32.lib";
    if (fs::exists(vcLibX86))
    {
        bat << " \"" << (vcLibX86 / "msvcrt.lib").string() << "\"";
        fs::path legacy = vcLibX86 / "legacy_stdio_definitions.lib";
        if (fs::exists(legacy))
            bat << " \"" << legacy.string() << "\"";
        fs::path vcruntime = vcLibX86 / "libvcruntime.lib";
        if (fs::exists(vcruntime))
            bat << " \"" << vcruntime.string() << "\"";
    }
    else
        bat << " msvcrt.lib";
    if (!sdkUcrtPath.empty())
        bat << " \"" << sdkUcrtPath << "\\ucrt.lib\"";
    bat << "\n";
    bat.close();
    if (!bat.good())
        return "";
    return "cmd /c \"" + batPath.string() + "\"";
}
#endif
