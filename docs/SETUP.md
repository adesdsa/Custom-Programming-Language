# picoC Setup Guide

This guide will help you set up everything needed to build and run picoC.

## Prerequisites

- **Windows 10/11**
- **Visual Studio 2019/2022** with C++ workload (or Build Tools for Visual Studio)
- **Git**

---

## Step 1: Install vcpkg (Package Manager)

vcpkg is a C/C++ package manager that handles our dependencies.

### 1.1 Clone vcpkg

Open PowerShell or Command Prompt and run:

```batch
cd C:\
git clone https://github.com/microsoft/vcpkg.git
```

### 1.2 Bootstrap vcpkg

```batch
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

### 1.3 Set Environment Variable

Add `VCPKG_ROOT` to your system environment variables:

**Option A: Using PowerShell (Admin)**
```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

**Option B: Using Command Prompt (Admin)**
```batch
setx VCPKG_ROOT "C:\vcpkg"
```

**Option C: Manually**
1. Press `Win + R`, type `sysdm.cpl`, press Enter
2. Go to **Advanced** → **Environment Variables**
3. Under **User variables**, click **New**
4. Variable name: `VCPKG_ROOT`
5. Variable value: `C:\vcpkg`
6. Click OK

> ⚠️ **Important**: Close and reopen your terminal after setting the environment variable.

### 1.4 Verify Installation

```batch
echo %VCPKG_ROOT%
```

Should output: `C:\vcpkg`

---

## Step 2: Install CMake

### Option A: Via Visual Studio
If you have Visual Studio installed, CMake is included. Make sure it's in your PATH.

### Option B: Standalone Installation
1. Download from: https://cmake.org/download/
2. During installation, select **"Add CMake to system PATH"**

### Verify Installation
```batch
cmake --version
```

---

## Step 3: Clone picoC

```batch
git clone <your-repo-url>
cd Custom-Programming-Language
```

---

## Step 4: Build the Project

The `picoc.bat` script handles everything automatically. vcpkg will download and install dependencies on first run.

### Available Commands

| Command | Description |
|---------|-------------|
| `picoc run [args]` | Build and run the compiler |
| `picoc test` | Build and run unit tests |
| `picoc release` | Build optimized release version |
| `picoc rebuild` | Clean and rebuild from scratch |
| `picoc help` | Show help message |

### First Build

```batch
picoc run
```

> ⏳ **Note**: The first build will take longer as vcpkg downloads and compiles dependencies.

---

## Dependencies

picoC uses the following libraries (automatically installed via vcpkg):

| Package | Description |
|---------|-------------|
| `mio` | Memory-mapped file I/O library |
| `gtest` | Google Test framework (for unit tests) |

---

## Troubleshooting

### "VCPKG_ROOT is not set"

Make sure you set the environment variable (Step 1.3) and **restarted your terminal**.

### "cmake is not recognized"

CMake is not in your PATH. Either:
- Install CMake standalone and add to PATH
- Use Visual Studio Developer Command Prompt

### "Visual Studio not found"

Make sure you have Visual Studio 2019 or 2022 with the **"Desktop development with C++"** workload installed.

### Build fails with missing dependencies

Run a clean rebuild:
```batch
picoc rebuild
```

### Still having issues?

Delete the build folder and try again:
```batch
rmdir /s /q build
picoc run
```

---
