$SDLVersion = "3.2.8"
$Cim = Get-CimInstance Win32_operatingsystem
$SysInfo = "Windows_" + $Cim.Version + "_" + $Cim.OSArchitecture -replace " ", "-"

if ($args.Count -eq 0) {
    Join-Path $PSScriptRoot "SDL_$SDLVersion-$SysInfo" | Write-Output
} elseif ($args[0] -eq "version") {
    Write-Output $SDLVersion
} elseif ($args[0] -eq "system") {
    Write-Output $SysInfo
}
