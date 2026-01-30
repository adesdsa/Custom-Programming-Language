@echo off
setlocal enabledelayedexpansion

set "COMMAND=%~1"
set "BUILD_DIR=%~dp0build"
set "EXE_NAME=Custom-Programming-Language.exe"
set "PROJECT_ROOT=%~dp0"

if "%COMMAND%"=="" set "COMMAND=help"

if /i "%COMMAND%"=="run" goto :run
if /i "%COMMAND%"=="test" goto :test
if /i "%COMMAND%"=="release" goto :release
if /i "%COMMAND%"=="debug" goto :debug
if /i "%COMMAND%"=="clean" goto :clean
if /i "%COMMAND%"=="help" goto :help
goto :help

:logo
echo.
echo    ######  #### ####### #######  ###### 
echo    ##   ##  ##  ##      ##   ## ##      
echo    ######   ##  ##      ##   ## ##      
echo    ##       ##  ##      ##   ## ##      
echo    ##      #### ####### #######  ###### 
echo.
echo    picoC - MIT License
echo.
goto :eof

:run
call :logo
echo.
echo   ^>^> Configuring Debug Build
echo.
cmake --preset debug
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR%" --config Debug
echo.
echo   ^>^> Running Project
echo.
"%BUILD_DIR%\Debug\%EXE_NAME%" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:test
call :logo
echo.
echo   ^>^> Configuring Debug Build
echo.
cmake --preset debug
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR%" --config Debug
echo.
echo   ^>^> Running Tests
echo.
ctest --test-dir "%BUILD_DIR%" -C Debug --output-on-failure
goto :eof

:release
call :logo
set "START_TIME=%TIME%"
echo.
echo   ^>^> Configuring Release Build
echo.
cmake --preset release
echo.
echo   ^>^> Building Release (optimized) - Full Rebuild
echo.
cmake --build "%BUILD_DIR%" --config Release --clean-first > "%BUILD_DIR%\release_build.log" 2>&1
type "%BUILD_DIR%\release_build.log"
set "END_TIME=%TIME%"

REM Count warnings and errors
set WARNING_COUNT=0
set ERROR_COUNT=0
for /f %%a in ('findstr /i /c:"warning C" /c:"warning:" "%BUILD_DIR%\release_build.log" 2^>nul ^| find /c /v ""') do set WARNING_COUNT=%%a
for /f %%a in ('findstr /i /c:"error C" /c:" error:" "%BUILD_DIR%\release_build.log" 2^>nul ^| find /c /v ""') do set ERROR_COUNT=%%a

REM Count source files
set CPP_COUNT=0
set H_COUNT=0
for /f %%a in ('dir /s /b "%PROJECT_ROOT%src\*.cpp" 2^>nul ^| find /c /v ""') do set CPP_COUNT=%%a
for /f %%a in ('dir /s /b "%PROJECT_ROOT%includes\*.h" 2^>nul ^| find /c /v ""') do set H_COUNT=%%a

REM ============================================
REM SHOW WARNINGS FIRST (if any)
REM ============================================
if %WARNING_COUNT% GTR 0 (
    echo.
    echo   +--------------------------------------------------------+
    echo   ^|                  COMPILER WARNINGS                     ^|
    echo   +--------------------------------------------------------+
    echo.
    findstr /i /c:"warning C" /c:"warning:" "%BUILD_DIR%\release_build.log" 2>nul
    echo.
)

REM ============================================
REM BUILD STATISTICS
REM ============================================
echo.
echo   +--------------------------------------------------------+
echo   ^|                   BUILD COMPLETE                       ^|
echo   +--------------------------------------------------------+
echo.

if %ERROR_COUNT% GTR 0 (
    echo   [XX] FAILED
) else if %WARNING_COUNT% GTR 0 (
    echo   [^^!^^!] SUCCESS with %WARNING_COUNT% warning^(s^)
) else (
    echo   [OK] SUCCESS
)
echo.
echo   +------------------+-------------------------------------+
echo   ^|  Configuration   ^|  Release ^(optimized^)                ^|
echo   ^|  Build Time      ^|  %START_TIME% - %END_TIME%          ^|
echo   +------------------+-------------------------------------+
echo   ^|  Source Files    ^|  %CPP_COUNT% .cpp / %H_COUNT% .h                    ^|
echo   ^|  Warnings        ^|  %WARNING_COUNT%                                 ^|
echo   ^|  Errors          ^|  %ERROR_COUNT%                                  ^|
echo   +------------------+-------------------------------------+

REM Check if executables exist and show sizes
if exist "%BUILD_DIR%\Release\%EXE_NAME%" (
    for %%F in ("%BUILD_DIR%\Release\%EXE_NAME%") do (
        set /a EXE_KB=%%~zF / 1024
        echo   ^|  Executable      ^|  !EXE_KB! KB                             ^|
    )
)
if exist "%BUILD_DIR%\Release\CPLib.lib" (
    for %%F in ("%BUILD_DIR%\Release\CPLib.lib") do (
        set /a LIB_KB=%%~zF / 1024
        echo   ^|  Library         ^|  !LIB_KB! KB                           ^|
    )
)
echo   +------------------+-------------------------------------+
echo.
goto :eof

:debug
call :logo
echo.
echo   ^>^> Configuring Debug Build
echo.
cmake --preset debug
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR%" --config Debug
goto :eof

:clean
call :logo
echo.
echo   ^>^> Cleaning Build Directory
echo.
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo   [OK] Build directory removed.
) else (
    echo   [--] Build directory doesn't exist.
)
echo.
goto :eof

:help
call :logo
echo   USAGE: build ^<command^> [args]
echo.
echo   --------------------------------------------------------
echo.
echo   run [args]     Build debug and run the executable
echo                  Example: build run input.txt
echo.
echo   test           Build debug and run all unit tests
echo.
echo   release        Build optimized release version
echo.
echo   debug          Build debug version only
echo.
echo   clean          Remove the build directory
echo.
echo   help           Show this help message
echo.
echo   --------------------------------------------------------
echo.
goto :eof
