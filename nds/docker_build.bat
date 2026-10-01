setlocal
title Silent Hill Nintendo DS - Clean Build Only
cd /d "%~dp0\.."

where docker >nul 2>nul
if errorlevel 1 (
    echo ERRORE: Docker non e' installato oppure non e' nel PATH.
    pause
    exit /b 1
)

docker info >nul 2>nul
if errorlevel 1 (
    echo ERRORE: Docker Desktop non e' avviato.
    pause
    exit /b 1
)

echo [1/2] Preparazione toolchain NDS...
docker run --rm -v "%cd%:/src" -w /src devkitpro/devkitarm:20260221 bash -lc "set -e; export DEVKITPRO=/opt/devkitpro; export DEVKITARM=/opt/devkitpro/devkitARM; export PATH=/opt/devkitpro/devkitARM/bin:/opt/devkitpro/tools/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin; unset CC CXX LD AR AS OBJCOPY; dkp-pacman -S --needed --noconfirm nds-dev"
if errorlevel 1 goto :fail

echo [2/2] Clean build ARM9...
docker run --rm -v "%cd%:/src" -w /src devkitpro/devkitarm:20260221 bash -lc "set -e; export DEVKITPRO=/opt/devkitpro; export DEVKITARM=/opt/devkitpro/devkitARM; export PATH=/opt/devkitpro/devkitARM/bin:/opt/devkitpro/tools/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin; unset CC CXX LD AR AS OBJCOPY; test -f /src/nds/tools/configure.py || { echo 'ERRORE: /src/nds/tools/configure.py non trovato'; exit 20; }; rm -rf /src/nds/build /src/nds/silent_hill.elf /src/nds/silent_hill.map /src/nds/silent_hill.nds; mkdir -p /src/nds/build; python3 /src/nds/tools/configure.py --root /src --out /src/nds/build/sources.mk; make -C /src/nds BUILD_READY=1 rom"
if errorlevel 1 goto :fail

echo.
echo BUILD COMPLETATA.
echo ROM: nds\silent_hill.nds
pause
exit /b 0

:fail
set ERR=%ERRORLEVEL%
echo.
echo BUILD FALLITA con codice %ERR%.
pause
exit /b %ERR%
