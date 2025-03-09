.\SDL3\buildSDL.ps1

Set-Location $PSScriptRoot

$SDLPath = .\SDL3\SDLpath.ps1
$Include = Join-Path $SDLPath include
$Lib = Join-Path $SDLPath lib
$Bin = Join-Path $SDLPath bin

clang.exe main.c "-I$Include" "-L$Lib" -lSDL3 -g -Wall -Wextra -Wpedantic -o build/main.exe

$env:Path = "$env:Path;$Bin"
.\build\main.exe
