<#
.SYNOPSIS
    Rebuilds the prebuilt GLFW and SoLoud libraries used by the Visual Studio
    solution, from exactly the same pinned sources as the CMake build.

.DESCRIPTION
    The Visual Studio solution (Processing_Sample/Processing_Sample.sln) links
    prebuilt static libraries instead of building GLFW and SoLoud itself. This
    script produces them with the repository's own CMake project, so the
    versions (and SHA-256-verified downloads) come from
    cmake/CProcessingDependencies.cmake - the single place dependency versions
    are pinned for every platform.

    Output (x64 and x86):
      Processing_Sample/CProcessing/GLFW/lib/<arch>/glfw3.lib       (Release, /MD - C only, also used by Debug)
      Processing_Sample/CProcessing/GLFW/inc/glfw3.h, glfw3native.h
      Processing_Sample/CProcessing/soloud/lib/<arch>/soloud.lib    (Release, /MD)
      Processing_Sample/CProcessing/soloud/lib/<arch>/soloud_d.lib  (Debug, /MDd)
      Processing_Sample/CProcessing/soloud/inc/soloud_c.h

    SoLoud (with miniaudio) is linked statically into CProcessing.dll, so
    there is no separate soloud.dll to ship.

    Requires Visual Studio 2022 or later with the C++ workload (its bundled
    CMake is found automatically) and an internet connection for the first
    configure.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\update-windows-prebuilt.ps1
#>
[CmdletBinding()]
param(
    [string]$BuildRoot = (Join-Path $env:TEMP "cprocessing-prebuilt"),
    [string]$Generator = ""
)
$ErrorActionPreference = "Stop"

$repo = Split-Path -Parent $PSScriptRoot
$cp = Join-Path $repo "Processing_Sample\CProcessing"

# Find CMake: PATH first, then the copy bundled with Visual Studio.
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    $cmake = Get-ChildItem "$env:ProgramFiles\Microsoft Visual Studio" -Recurse -Filter cmake.exe -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "CommonExtensions\\Microsoft\\CMake" } | Select-Object -First 1 -ExpandProperty FullName
}
if (-not $cmake) { throw "CMake not found. Install Visual Studio's C++ workload or CMake 3.20+." }
Write-Host "Using $cmake"

$genArgs = @()
if ($Generator) { $genArgs = @("-G", $Generator) }

foreach ($arch in @(@{ Name = "x64"; Cmake = "x64" }, @{ Name = "x86"; Cmake = "Win32" })) {
    $build = Join-Path $BuildRoot $arch.Name
    Write-Host "`n=== $($arch.Name): configuring in $build"
    & $cmake -S $repo -B $build @genArgs -A $arch.Cmake `
        -DCPROCESSING_BUILD_DEMOS=OFF -DCPROCESSING_BUILD_TESTS=OFF `
        "-DCMAKE_C_FLAGS_DEBUG=/Od /RTC1" "-DCMAKE_CXX_FLAGS_DEBUG=/Od /RTC1"
    if ($LASTEXITCODE) { throw "configure failed ($($arch.Name))" }

    # Debug libraries are built without debug info so they don't reference a
    # .pdb that isn't shipped (LNK4099); step into CProcessing, not SoLoud.
    foreach ($config in "Release", "Debug") {
        Write-Host "=== $($arch.Name) ${config}: building glfw + soloud"
        & $cmake --build $build --config $config --target glfw soloud --parallel
        if ($LASTEXITCODE) { throw "build failed ($($arch.Name) $config)" }
    }

    $glfwLibDir = Join-Path $cp "GLFW\lib\$($arch.Name)"
    $soloudLibDir = Join-Path $cp "soloud\lib\$($arch.Name)"
    New-Item -ItemType Directory -Force $glfwLibDir, $soloudLibDir | Out-Null

    Copy-Item (Join-Path $build "lib\Release\glfw3.lib") $glfwLibDir -Force
    Copy-Item (Join-Path $build "lib\Release\soloud.lib") (Join-Path $soloudLibDir "soloud.lib") -Force
    Copy-Item (Join-Path $build "lib\Debug\soloud.lib") (Join-Path $soloudLibDir "soloud_d.lib") -Force
}

# Headers (identical for both architectures)
$x64 = Join-Path $BuildRoot "x64"
Copy-Item (Join-Path $x64 "_deps\glfw-src\include\GLFW\glfw3.h") (Join-Path $cp "GLFW\inc") -Force
Copy-Item (Join-Path $x64 "_deps\glfw-src\include\GLFW\glfw3native.h") (Join-Path $cp "GLFW\inc") -Force
Copy-Item (Join-Path $x64 "_deps\glfw-src\LICENSE.md") (Join-Path $cp "GLFW") -Force
Copy-Item (Join-Path $x64 "_deps\soloud-src\include\soloud_c.h") (Join-Path $cp "soloud\inc") -Force
Copy-Item (Join-Path $x64 "_deps\soloud-src\LICENSE") (Join-Path $cp "soloud") -Force

# Record what was built, read straight from the pins
$deps = Get-Content (Join-Path $repo "cmake\CProcessingDependencies.cmake") -Raw
function Get-Pin($name) { if ($deps -match "set\($name `"([^`"]+)`"\)") { $Matches[1] } else { "?" } }
$date = Get-Date -Format "yyyy-MM-dd"
@"
GLFW $(Get-Pin 'CPROCESSING_GLFW_VERSION')
Built $date by tools/update-windows-prebuilt.ps1 from the pins in cmake/CProcessingDependencies.cmake
(static library, Release, /MD; used by both Debug and Release builds of CProcessing)
"@ | Set-Content (Join-Path $cp "GLFW\Version.txt") -Encoding ascii
@"
SoLoud commit $(Get-Pin 'CPROCESSING_SOLOUD_COMMIT') with miniaudio $(Get-Pin 'CPROCESSING_MINIAUDIO_VERSION')
Built $date by tools/update-windows-prebuilt.ps1 from the pins in cmake/CProcessingDependencies.cmake
(static libraries: soloud.lib Release /MD, soloud_d.lib Debug /MDd; backends: miniaudio + nosound)
"@ | Set-Content (Join-Path $cp "soloud\Version.txt") -Encoding ascii

Write-Host "`nDone. Rebuild Processing_Sample.sln (all four configurations) and run both test projects."
