@echo off
setlocal

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && (set ARCH=x86) || (set ARCH=x64)

set CC=C:\MinGW\bin\gcc.exe

set SDL=C:\Users\ihebl\OneDrive\Bureau\ING1\Atelier de Programmation (C)\Project\flappy-game\lib\SDL2-2.30.9-win32-x64
set TTF=C:\Users\ihebl\OneDrive\Bureau\ING1\Atelier de Programmation (C)\Project\flappy-game\lib\SDL2_ttf-2.0.12-win32-x64

echo Architecture: %ARCH%
echo Compiler Path: %CC%

:: Add the folders containing the SDL2 and SDL2_ttf DLLs to PATH
set PATH=%PATH%;%SDL%;%TTF%

if not exist "%CC%" (
    echo Compiler not found at %CC%
    pause
    exit /b 1
)

age with .lib files.
"%CC%" -DSDL_MAIN_HANDLED ^
    -I"%SDL%\include" ^
    -I"%TTF%\include" ^
    flappy.c -o flappy.exe ^
    -lSDL2 -lSDL2_ttf

pause