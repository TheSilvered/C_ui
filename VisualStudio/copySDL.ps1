$SDLSource = .\..\SDL3\SDLpath.ps1
$Lib = Join-Path $SDLSource "lib"
$Dll = Join-Path $SDLSource "bin\SDL3.dll"

Copy-Item -Path $Lib -Destination (Join-Path $args[0] "SDL3") -Recurse -Force
Copy-Item -Path $Dll -Destination $args[0] -Force
