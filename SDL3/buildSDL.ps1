Set-Location $PSScriptRoot

$InstallPath = .\SDLpath.ps1

if (Test-Path "$InstallPath") {
    exit 0
}

$SDLVersion = .\SDLpath.ps1 version
$SysInfo = .\SDLpath.ps1 system

$SrcDir = "SDL"
$GitRepo = "https://github.com/libsdl-org/SDL.git"

# Clone the source code if needed
if (Test-Path "$srcDir") {
    Set-Location "$SrcDir"
    git checkout "release-$SDLVersion"
} else {
    git clone --depth 1 -b "release-$SDLVersion" "$GitRepo" "$SrcDir"
    Set-Location "$SrcDir"
}

Write-Output "Building SDL $SDLVersion for $SysInfo"

if (Test-Path "build") {
    Remove-Item -LiteralPath "build" -Recurse -Force
}
New-Item "build" -ItemType Directory > $null
Set-Location "build"
cmake ..
cmake --build . --config RelWithDebInfo
cmake --install . --config RelWithDebInfo --prefix "$InstallPath"
