$IncludeDir="include"
$LibDir="lib"
$TmpFolder="temporary"
$Raylib="raylib-5.5_win64_mingw-w64"
$RaylibZip="$Raylib.zip"

function Get-Dependencies {
    Write-Host "Downloading dependencies..."
    If (-Not (Test-Path "$TmpFolder")) {
        New-Item -Path "$TmpFolder" -ItemType Directory | Out-Null
    }

    If (-Not (Test-Path "$LibDir")) {
        New-Item -Path "$LibDir" -ItemType Directory | Out-Null
    }

    If (-Not (Test-Path "$IncludeDir")) {
        New-Item -Path "$IncludeDir" -ItemType Directory | Out-Null
    }

    Invoke-WebRequest "https://github.com/raysan5/raylib/releases/download/5.5/$RaylibZip" -OutFile "$TmpFolder\$RaylibZip"
    Invoke-WebRequest "https://raw.githubusercontent.com/raysan5/raylib/refs/heads/master/src/external/rprand.h" -OutFile "$TmpFolder\rprand.h"

    Expand-Archive "$TmpFolder\$RaylibZip" -DestinationPath "$TmpFolder" -Force

    Move-Item "$TmpFolder\$Raylib\include\*" "$IncludeDir" -Force
    Move-Item "$TmpFolder\$Raylib\lib\*" "$LibDir" -Force

    If (-Not (Test-Path "$IncludeDir\external")) {
        New-Item "$IncludeDir\external" -ItemType Directory | Out-Null
    }

    Move-Item "$TmpFolder\rprand.h" "$IncludeDir\external" -Force

    Remove-Item "$TmpFolder" -Recurse

    Write-Host "Done downloading dependencies"
}

If (
    (-Not (Test-Path "$LibDir\libraylib.a")) -or
    (-Not (Test-Path "$LibDir\libraylibdll.a")) -or
    (-Not (Test-Path "$LibDir\raylib.dll")) -or
    (-Not (Test-Path "$IncludeDir\raylib.h")) -or
    (-Not (Test-Path "$IncludeDir\rlgl.h")) -or
    (-Not (Test-Path "$IncludeDir\raymath.h")) -or
    (-Not (Test-Path "$IncludeDir\external\rprand.h"))
) {
    Get-Dependencies
} Else {
    Write-Host "All good to go!"
}

Write-Host "Done!"