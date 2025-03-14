@echo off
title Just a second...
echo Downloading dependencies...

set includedir=include
set libdir=lib

:: :/ wanted to do if-else, but cmd said "the syntax of the command is incorrect"
if not exist %libdir%\libraylib.a goto exec
if not exist %libdir%\libraylibdll.a goto exec
if not exist %libdir%\raylib.dll goto exec
if not exist %includedir%\raylib.h goto exec
if not exist %includedir%\raymath.h goto exec
if not exist %includedir%\rlgl.h goto exec
if not exist %includedir%\external\rprand.h goto exec
goto done

:exec
set tmp=temporary
set raylib=raylib-5.5_win64_mingw-w64
set raylibzip=%raylib%.zip
    
md %tmp%

if not exist %includedir% (
    md %includedir%
)

if not exist %libdir% (
    md %libdir%
)
wget -c -P %tmp% https://github.com/raysan5/raylib/releases/download/5.5/%raylibzip%
wget -c -P %tmp% https://raw.githubusercontent.com/raysan5/raylib/refs/heads/master/src/external/rprand.h

echo Done downloading dependencies!
echo Moving dependencies to the correct folders...

:: move them to other folders

tar -xvf %tmp%\%raylibzip% -C %tmp%
move %tmp%\%raylib%\lib\* %libdir%
move %tmp%\%raylib%\include\*.h %includedir%

if not exist %includedir%\external md %includedir%\external
move %tmp%\rprand.h %includedir%\external

 :: delete %tmp% folder and everything inside it

rm -rf %tmp%

:done
echo Done!
:: download raylib 5.5 and rprand.h to the created %tmp% folder

