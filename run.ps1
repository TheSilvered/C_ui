.\SDL3\buildSDL.ps1

Set-Location $PSScriptRoot

if (!(Test-Path build)) {
    New-Item "build" -ItemType Directory
}

$SDLPath = .\SDL3\SDLpath.ps1
$Include = Join-Path $SDLPath include
$Lib = Join-Path $SDLPath lib
$Bin = Join-Path $SDLPath bin
$Flags = @('-g', '-Wall', '-Wextra', '-Wpedantic')

clang.exe main.c "-I$Include" "-L$Lib" -lSDL3 $Flags -o build/main.exe

if ($LASTEXITCODE -eq 0) {
    $env:Path = "$env:Path;$Bin"
    .\build\main.exe
}
