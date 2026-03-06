# Platform Features Test - Windows
# Run from project root: .\tests\run_platform_features_test.ps1
# Requires: compiler built at .\build_test\Debug\Custom-Programming-Language.exe

$ErrorActionPreference = "Continue"
$compiler = ".\build_test\Debug\Custom-Programming-Language.exe"
$outDir = ".\build_test\Debug"
$testExe = "$outDir\platform_features_test.exe"

Write-Host "`nPlatform Features Test" -ForegroundColor Cyan
Write-Host "======================" -ForegroundColor Cyan

& $compiler -o platform_features_test --out-dir $outDir tests\inputs\platform_features_test.picoc 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Compile FAILED" -ForegroundColor Red
    exit 1
}

$stdin = "hello`n42`n3.14"
$output = $stdin | & $testExe 2>&1 | Out-String
$exitCode = $LASTEXITCODE

Write-Host "Exit code: $exitCode" -ForegroundColor $(if ($exitCode -eq 0) { "Green" } else { "Red" })
if ($exitCode -eq 0) {
    Write-Host "PASS - All features exercised" -ForegroundColor Green
} else {
    Write-Host "FAIL" -ForegroundColor Red
}
exit $exitCode
