@ECHO OFF

ECHO "Cleaning everything..."

REM Engine

make -f "Makefile.vulkan.windows.mak" clean
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
