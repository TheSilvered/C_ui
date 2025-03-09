$SDLSource = .\..\SDL3\SDLpath.ps1
$ExeLocation = Join-Path $PSScriptRoot $args[0]
$Lib = Join-Path $SDLSource "lib"
$Dll = Join-Path $SDLSource "bin\SDL3.dll"

Copy-Item -Path $Lib -Destination (Join-Path $ExeLocation "SDL3") -Recurse -Force
Copy-Item -Path $Dll -Destination $ExeLocation -Force
