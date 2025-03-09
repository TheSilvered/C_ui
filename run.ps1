.\SDL3\buildSDL.ps1

Set-Location $PSScriptRoot

# Clean build directory
if (Test-Path "build") {
    Remove-Item -LiteralPath "build" -Recurse -Force
}
New-Item "build" -ItemType Directory > $null

$SDLPath = .\SDL3\SDLpath.ps1
$Include = Join-Path $SDLPath include
$Lib = Join-Path $SDLPath lib
$Bin = Join-Path $SDLPath bin

clang.exe main.c "-I$Include" "-L$Lib" -lSDL3 -g -Wall -Wextra -Wpedantic -o build/main.exe

Move-Item -Path build/main.exe -Destination main.exe -Force
$env:Path = "$env:Path;$Bin"
./main.exe
