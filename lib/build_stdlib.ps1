# Build picoC standard library pre-compiled objects
# Run from the repository root: powershell -File lib/build_stdlib.ps1

param(
    [string]$NasmDir = "tools/nasm"
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not $RepoRoot) { $RepoRoot = (Get-Location).Path }

$Nasm64 = Join-Path $RepoRoot "$NasmDir/nasm64.exe"
$Nasm32 = Join-Path $RepoRoot "$NasmDir/nasm32.exe"
$StdlibSrc = Join-Path $RepoRoot "lib/stdlib"
$OutDir = Join-Path $RepoRoot "lib"

function Build-Asm($nasm, $format, $src, $out) {
    Write-Host "  $src -> $out"
    & $nasm -f $format $src -o $out
    if ($LASTEXITCODE -ne 0) { throw "NASM failed for $src" }
}

Write-Host "Building picoC stdlib..."

if (Test-Path $Nasm64) {
    Build-Asm $Nasm64 "win64"  "$StdlibSrc/string_win64.asm"   "$OutDir/string_win64.obj"
    Build-Asm $Nasm64 "elf64"  "$StdlibSrc/string_linux64.asm"  "$OutDir/string_linux64.o"
    Build-Asm $Nasm64 "win64"  "$StdlibSrc/math_win64.asm"     "$OutDir/math_win64.obj"
    Build-Asm $Nasm64 "elf64"  "$StdlibSrc/math_linux64.asm"    "$OutDir/math_linux64.o"
}

if (Test-Path $Nasm32) {
    Build-Asm $Nasm32 "win32"  "$StdlibSrc/string_win32.asm"   "$OutDir/string_win32.obj"
    Build-Asm $Nasm32 "elf32"  "$StdlibSrc/string_linux32.asm"  "$OutDir/string_linux32.o"
    Build-Asm $Nasm32 "win32"  "$StdlibSrc/math_win32.asm"     "$OutDir/math_win32.obj"
    Build-Asm $Nasm32 "elf32"  "$StdlibSrc/math_linux32.asm"    "$OutDir/math_linux32.o"
}

Write-Host "Done."
