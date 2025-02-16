@echo off
title Just a second...
echo Downloading dependencies...

set tmp=temporary
set includedir=include
set libdir=lib

set raylib=raylib-5.5_win64_mingw-w64
set raylibzip=%raylib%.zip

md %tmp%

if not exist %includedir% (
    md %includedir%
)

if not exist %libdir% (
    md %libdir%
)

:: download raylib 5.5 and rprand.h to the created %tmp% folder

wget -c -P %tmp% https://github.com/raysan5/raylib/releases/download/5.5/%raylibzip%
wget -c -P %tmp% https://raw.githubusercontent.com/raysan5/raylib/refs/heads/master/src/external/rprand.h

echo Done downloading dependencies!
echo Moving dependencies to the correct folders...

:: move them to other folders

tar -xvf %tmp%\%raylibzip% -C %tmp%
move %tmp%\%raylib%\lib\* %libdir%
move %tmp%\%raylib%\include\*.h %includedir%

md %includedir%\external
move %tmp%\rprand.h %includedir%\external

rm -rf %tmp%

echo Done!