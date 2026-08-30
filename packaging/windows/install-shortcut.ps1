<#
    Create (or remove) Start Menu and Desktop shortcuts for ETP.

    Normally invoked by double-clicking "Install Shortcut.bat", which just
    forwards to this file. The logic lives here rather than inline in the
    batch file because embedding PowerShell in .bat requires caret line
    continuations and nested quote escaping, which is easy to get subtly
    wrong and hard to debug.

    Usage:
        .\install-shortcut.ps1            Install shortcuts
        .\install-shortcut.ps1 -Remove    Delete them again
#>

[CmdletBinding()]
param(
    [switch]$Remove
)

$ErrorActionPreference = 'Stop'

# Shortcuts must point at the game where it actually lives, so everything
# resolves relative to this script rather than the caller's location.
$GameDir = $PSScriptRoot
$Exe     = Join-Path $GameDir 'ETP.exe'

$Shortcuts = @(
    Join-Path ([Environment]::GetFolderPath('Programs')) 'Eden Treaty Pandemonium.lnk'
    Join-Path ([Environment]::GetFolderPath('Desktop'))  'Eden Treaty Pandemonium.lnk'
)

if ($Remove) {
    foreach ($path in $Shortcuts) {
        if (Test-Path $path) {
            Remove-Item -Force $path
            Write-Host "Removed $path"
        }
    }
    Write-Host 'Done.'
    exit 0
}

if (-not (Test-Path $Exe)) {
    Write-Host "Could not find ETP.exe next to this script ($GameDir)."
    Write-Host 'Keep this script in the same folder as the game.'
    exit 1
}

$shell = New-Object -ComObject WScript.Shell
foreach ($path in $Shortcuts) {
    $link = $shell.CreateShortcut($path)
    $link.TargetPath = $Exe
    # Without this the game launches with the wrong working directory and
    # fails to find assets\.
    $link.WorkingDirectory = $GameDir
    $link.IconLocation = $Exe
    $link.Description = 'Bullet-hell shoot-em-up'
    $link.Save()
    Write-Host "Created $path"
}

Write-Host 'Done.'
