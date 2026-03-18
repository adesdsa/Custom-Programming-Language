@echo off
setlocal enabledelayedexpansion

:: environment variables
set "COMMAND=%~1"
set "BUILD_DIR=%~dp0build"
set "EXE_NAME=Custom-Programming-Language.exe"
set "PROJECT_ROOT=%~dp0"

:: Define colors
for /f %%a in ('echo prompt $E^| cmd') do set "ESC=%%a"

:: Color definitions
set "RED=%ESC%[91m"
set "GREEN=%ESC%[92m"
set "YELLOW=%ESC%[93m"
set "BLUE=%ESC%[94m"
set "CYAN=%ESC%[96m"
set "RESET=%ESC%[0m"

:: forward to help command if no arguments have been pass
if "%COMMAND%"=="" set "COMMAND=help"

:: available commands
if /i "%COMMAND%"=="run" goto :run
if /i "%COMMAND%"=="test" goto :test
if /i "%COMMAND%"=="test32" goto :test32
if /i "%COMMAND%"=="release" goto :release
if /i "%COMMAND%"=="rebuild" goto :rebuild
if /i "%COMMAND%"=="help" goto :help
goto :help 
:: case of unknown command

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

:: run command implementation
:run
call :logo
echo.
echo   ^>^> Configuring Debug Build
echo.
cmake --preset run
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR%" --config Debug
echo.
echo   ^>^> Running Project
echo.
"%BUILD_DIR%\Debug\%EXE_NAME%" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:: test command implementation
:test
call :logo
echo.
echo   ^>^> Configuring Debug Build (win64)
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

:: test32 command implementation (Win32)
:test32
call :logo
echo.
echo   ^>^> Configuring Debug Build (win32)
echo.
set "BUILD_DIR32=%~dp0build32"
cmake --preset debug32
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR32%" --config Debug
echo.
echo   ^>^> Running Tests
echo.
ctest --test-dir "%BUILD_DIR32%" -C Debug --output-on-failure
goto :eof

:: release command implementation
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

:: count warnings and errors
set WARNING_COUNT=0
set ERROR_COUNT=0
for /f %%a in ('findstr /i /c:"warning C" /c:"warning:" "%BUILD_DIR%\release_build.log" 2^>nul ^| find /c /v ""') do set WARNING_COUNT=%%a
for /f %%a in ('findstr /i /c:"error C" /c:" error:" "%BUILD_DIR%\release_build.log" 2^>nul ^| find /c /v ""') do set ERROR_COUNT=%%a

:: count source files
set CPP_COUNT=0
set H_COUNT=0
for /f %%a in ('dir /s /b "%PROJECT_ROOT%src\*.cpp" 2^>nul ^| find /c /v ""') do set CPP_COUNT=%%a
for /f %%a in ('dir /s /b "%PROJECT_ROOT%includes\*.h" 2^>nul ^| find /c /v ""') do set H_COUNT=%%a

:: display warnnings
if %WARNING_COUNT% GTR 0 (
    echo.
    echo   +--------------------------------------------------------+
    echo   ^|                  COMPILER WARNINGS                     ^|
    echo   +--------------------------------------------------------+
    echo.
    findstr /i /c:"warning C" /c:"warning:" "%BUILD_DIR%\release_build.log" 2>nul
    echo.
)

:: determine build status
if %ERROR_COUNT% GTR 0 (
    set "STATUS=[XX] FAILED"
) else if %WARNING_COUNT% GTR 0 (
    set "STATUS=[^!^!] SUCCESS with %WARNING_COUNT% warning(s)"
) else (
    set "STATUS=[OK] SUCCESS"
)

:: display statistics
echo.
echo   +--------------------------------------------------------+
echo   ^|                   BUILD COMPLETE                       ^|
echo   +------------------+-------------------------------------+
echo   ^|  Status          ^|  !STATUS!    ^|
echo   ^|  Configuration   ^|  Release ^(optimized^)                ^|
echo   ^|  Build Time      ^|  %START_TIME% - %END_TIME%          ^|
echo   +------------------+-------------------------------------+
echo   ^|  Source Files    ^|  %CPP_COUNT% .cpp / %H_COUNT% .h                    ^|
echo   ^|  Warnings        ^|  %WARNING_COUNT%                                 ^|
echo   ^|  Errors          ^|  %ERROR_COUNT%                                  ^|
echo   +------------------+-------------------------------------+

:: check if executables exist and show sizes
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

:rebuild
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
echo   ^>^> Reconfiguring Project
echo.
cmake --preset run
echo.
echo   ^>^> Building Debug
echo.
cmake --build "%BUILD_DIR%" --config Debug
echo.
echo   [OK] Rebuild complete.
echo.
goto :eof

:help
call :logo
echo   USAGE: build ^<command^> [args]
echo.
echo   --------------------------------------------------------
echo.
echo   run [args]     Build debug and run the executable
echo                  Example: picoc run input.txt
echo.
echo   test           Build with tests and run all unit tests (win64)
echo.
echo   test32         Build and run tests for Win32 (x86)
echo                  Requires VCPKG_ROOT set
echo.
echo   release        Build optimized release version
echo.
echo   rebuild        Clean and rebuild from scratch
echo.
echo   help           Show this help message
echo.
echo   --------------------------------------------------------
echo.
goto :eof
