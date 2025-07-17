!/bin/bash
# Build script for rebuilding everything
set echo on

echo "Building everything..."


# pushd engine
# source build.sh
# popd
make -f Makefile.vulkan.linux.wayland.mak clean

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "All assemblies cleaned successfully."