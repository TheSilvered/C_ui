# Clean build directory
if (Test-Path -Path build) {
    Remove-Item -Path "build/*" -Recurse -Force
} else {
    New-Item -Path build -ItemType Directory
}

clang.exe main.c -ISDL3/include -LSDL3/lib -lSDL3 -g -Wall -Wextra -Wpedantic -o build/main.exe
Move-Item -Path build/main.exe -Destination main.exe -Force
$env:Path = "$env:Path;SDL3/bin"
./main.exe
