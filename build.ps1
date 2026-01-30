param(
    [Parameter(Position=0)]
    [string]$Command = "help",
    [Parameter(Position=1, ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

$ProjectRoot = $PSScriptRoot
$BuildDir = "$ProjectRoot\build"
$ExeName = "Custom-Programming-Language.exe"

function Show-Logo {
    Write-Host ""
    Write-Host "   " -NoNewline
    Write-Host "######" -NoNewline -ForegroundColor Cyan
    Write-Host "  " -NoNewline
    Write-Host "####" -NoNewline -ForegroundColor Yellow
    Write-Host " " -NoNewline
    Write-Host "#######" -NoNewline -ForegroundColor Cyan
    Write-Host " " -NoNewline
    Write-Host "#######" -NoNewline -ForegroundColor Cyan
    Write-Host "  " -NoNewline
    Write-Host "######" -ForegroundColor Yellow
    
    Write-Host "   " -NoNewline
    Write-Host "##   ##" -NoNewline -ForegroundColor Cyan
    Write-Host "  " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Yellow
    Write-Host "  " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Cyan
    Write-Host "      " -NoNewline
    Write-Host "##   ##" -NoNewline -ForegroundColor Cyan
    Write-Host " " -NoNewline
    Write-Host "##" -ForegroundColor Yellow
    
    Write-Host "   " -NoNewline
    Write-Host "######" -NoNewline -ForegroundColor Cyan
    Write-Host "   " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Yellow
    Write-Host "  " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Cyan
    Write-Host "      " -NoNewline
    Write-Host "##   ##" -NoNewline -ForegroundColor Cyan
    Write-Host " " -NoNewline
    Write-Host "##" -ForegroundColor Yellow
    
    Write-Host "   " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Cyan
    Write-Host "       " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Yellow
    Write-Host "  " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Cyan
    Write-Host "      " -NoNewline
    Write-Host "##   ##" -NoNewline -ForegroundColor Cyan
    Write-Host " " -NoNewline
    Write-Host "##" -ForegroundColor Yellow
    
    Write-Host "   " -NoNewline
    Write-Host "##" -NoNewline -ForegroundColor Cyan
    Write-Host "      " -NoNewline
    Write-Host "####" -NoNewline -ForegroundColor Yellow
    Write-Host " " -NoNewline
    Write-Host "#######" -NoNewline -ForegroundColor Cyan
    Write-Host " " -NoNewline
    Write-Host "#######" -NoNewline -ForegroundColor Cyan
    Write-Host "  " -NoNewline
    Write-Host "######" -ForegroundColor Yellow
    
    Write-Host ""
    Write-Host "   picoC - MIT License" -ForegroundColor DarkGray
    Write-Host ""
}

function Show-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "  >> $Message" -ForegroundColor Yellow
    Write-Host ""
}

function Invoke-DebugBuild {
    Show-Step "Configuring Debug Build"
    cmake --preset debug
    Show-Step "Building Debug"
    cmake --build $BuildDir --config Debug
}

function Invoke-ReleaseBuild {
    Show-Logo
    $StartTime = Get-Date
    
    Show-Step "Configuring Release Build"
    cmake --preset release
    
    Show-Step "Building Release (optimized) - Full Rebuild"
    $null = New-Item -ItemType Directory -Force -Path $BuildDir -ErrorAction SilentlyContinue
    cmake --build $BuildDir --config Release --clean-first 2>&1 | Tee-Object -FilePath "$BuildDir\release_build.log"
    
    $EndTime = Get-Date
    $Duration = $EndTime - $StartTime
    
    # Parse build log
    $LogPath = "$BuildDir\release_build.log"
    $WarningCount = 0
    $ErrorCount = 0
    $Warnings = @()
    
    if (Test-Path $LogPath) {
        $LogContent = Get-Content $LogPath -Raw
        $Warnings = Select-String -Path $LogPath -Pattern "warning C\d+|warning:" -AllMatches | ForEach-Object { $_.Line }
        $WarningCount = $Warnings.Count
        $ErrorCount = (Select-String -Path $LogPath -Pattern "error C\d+|error:" -AllMatches | Measure-Object).Count
    }
    
    # Get sizes
    $ExePath = "$BuildDir\Release\$ExeName"
    $LibPath = "$BuildDir\Release\CPLib.lib"
    
    $ExeSize = 0
    $LibSize = 0
    if (Test-Path $ExePath) { $ExeSize = (Get-Item $ExePath).Length }
    if (Test-Path $LibPath) { $LibSize = (Get-Item $LibPath).Length }
    
    $ExeSizeStr = if ($ExeSize -ge 1MB) { "{0:N2} MB" -f ($ExeSize / 1MB) } elseif ($ExeSize -ge 1KB) { "{0:N2} KB" -f ($ExeSize / 1KB) } else { "N/A" }
    $LibSizeStr = if ($LibSize -ge 1MB) { "{0:N2} MB" -f ($LibSize / 1MB) } elseif ($LibSize -ge 1KB) { "{0:N2} KB" -f ($LibSize / 1KB) } else { "N/A" }
    
    # Count files
    $CppFiles = (Get-ChildItem -Path "$ProjectRoot\src" -Filter "*.cpp" -Recurse -ErrorAction SilentlyContinue | Measure-Object).Count
    $HeaderFiles = (Get-ChildItem -Path "$ProjectRoot\includes" -Filter "*.h" -Recurse -ErrorAction SilentlyContinue | Measure-Object).Count
    
    # Show warnings first
    if ($WarningCount -gt 0) {
        Write-Host ""
        Write-Host "  +----------------------------------------------------------+" -ForegroundColor Yellow
        Write-Host "  |                  COMPILER WARNINGS                       |" -ForegroundColor Yellow
        Write-Host "  +----------------------------------------------------------+" -ForegroundColor Yellow
        Write-Host ""
        $Warnings | Select-Object -First 15 | ForEach-Object {
            $short = if ($_.Length -gt 70) { $_.Substring(0, 67) + "..." } else { $_ }
            Write-Host "  $short" -ForegroundColor Gray
        }
        if ($WarningCount -gt 15) {
            Write-Host ""
            Write-Host "  ... and $($WarningCount - 15) more warnings" -ForegroundColor DarkYellow
        }
        Write-Host ""
    }
    
    # Show summary
    Write-Host ""
    Write-Host "  +----------------------------------------------------------+" -ForegroundColor Cyan
    Write-Host "  |                   BUILD COMPLETE                         |" -ForegroundColor Cyan
    Write-Host "  +----------------------------------------------------------+" -ForegroundColor Cyan
    Write-Host ""
    
    # Status
    if ($ErrorCount -gt 0) {
        Write-Host "    [XX] FAILED" -ForegroundColor Red
    } elseif ($WarningCount -gt 0) {
        Write-Host "    [!!] SUCCESS with $WarningCount warning(s)" -ForegroundColor Yellow
    } else {
        Write-Host "    [OK] SUCCESS" -ForegroundColor Green
    }
    Write-Host ""
    
    # Stats
    Write-Host "  +------------------+---------------------------------------+" -ForegroundColor DarkGray
    Write-Host "  | Configuration    | Release (optimized)                   |" -ForegroundColor DarkGray
    Write-Host "  | Build Time       | $($Duration.ToString('mm\:ss'))                                |" -ForegroundColor DarkGray
    Write-Host "  +------------------+---------------------------------------+" -ForegroundColor DarkGray
    Write-Host "  | Source Files     | $CppFiles .cpp  |  $HeaderFiles .h                        |" -ForegroundColor DarkGray
    Write-Host -NoNewline "  | Warnings         | " -ForegroundColor DarkGray
    if ($WarningCount -gt 0) { Write-Host -NoNewline "$WarningCount" -ForegroundColor Yellow } else { Write-Host -NoNewline "0" -ForegroundColor Green }
    Write-Host "                                      |" -ForegroundColor DarkGray
    Write-Host -NoNewline "  | Errors           | " -ForegroundColor DarkGray
    if ($ErrorCount -gt 0) { Write-Host -NoNewline "$ErrorCount" -ForegroundColor Red } else { Write-Host -NoNewline "0" -ForegroundColor Green }
    Write-Host "                                      |" -ForegroundColor DarkGray
    Write-Host "  +------------------+---------------------------------------+" -ForegroundColor DarkGray
    Write-Host "  | Executable       | $ExeSizeStr                                 |" -ForegroundColor DarkGray
    Write-Host "  | Library          | $LibSizeStr                                |" -ForegroundColor DarkGray
    Write-Host "  +------------------+---------------------------------------+" -ForegroundColor DarkGray
    Write-Host ""
}

switch ($Command) {
    "run" {
        Show-Logo
        Invoke-DebugBuild
        Show-Step "Running Project"
        $exe = "$BuildDir\Debug\$ExeName"
        if (Test-Path $exe) { & $exe @Arguments }
        else { Write-Error "Executable not found: $exe" }
    }
    "test" {
        Show-Logo
        Invoke-DebugBuild
        Show-Step "Running Tests"
        ctest --test-dir $BuildDir -C Debug --output-on-failure
    }
    "release" {
        Invoke-ReleaseBuild
    }
    "debug" {
        Show-Logo
        Invoke-DebugBuild
    }
    "clean" {
        Show-Logo
        Show-Step "Cleaning Build Directory"
        if (Test-Path $BuildDir) {
            Remove-Item -Recurse -Force $BuildDir
            Write-Host "  [OK] Build directory removed." -ForegroundColor Green
        } else {
            Write-Host "  [--] Build directory does not exist." -ForegroundColor Yellow
        }
        Write-Host ""
    }
    default {
        Show-Logo
        Write-Host "  USAGE: .\build.ps1 <command> [args]" -ForegroundColor White
        Write-Host ""
        Write-Host "  --------------------------------------------------------" -ForegroundColor DarkGray
        Write-Host ""
        Write-Host "  run [args]     Build debug and run the executable" -ForegroundColor White
        Write-Host "  test           Build debug and run all unit tests" -ForegroundColor White
        Write-Host "  release        Build optimized release version" -ForegroundColor White
        Write-Host "  debug          Build debug version only" -ForegroundColor White
        Write-Host "  clean          Remove the build directory" -ForegroundColor White
        Write-Host "  help           Show this help message" -ForegroundColor White
        Write-Host ""
        Write-Host "  --------------------------------------------------------" -ForegroundColor DarkGray
        Write-Host ""
    }
}
