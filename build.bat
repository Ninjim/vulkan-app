@ECHO OFF
REM Build Everything

ECHO "Building everything..."

REM Engine

ECHO "Building engine"

make -f "Makefile.vulkan.windows.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)