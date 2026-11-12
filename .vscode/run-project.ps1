$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot "build"

& cmake `
    -S $projectRoot `
    -B $buildDirectory `
    -G "MinGW Makefiles" `
    -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe `
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe `
    -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& cmake --build $buildDirectory --parallel 4
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
Set-Location $buildDirectory
& (Join-Path $buildDirectory "terrain_simulator.exe")
exit $LASTEXITCODE
