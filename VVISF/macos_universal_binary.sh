#!/bin/bash

echo "************** Beginning to build universal binary for macOS"

export MACOSX_DEPLOYMENT_TARGET=10.11


# clean whatever's there
make --f Makefile clean

# set the env var for the x86_64 build, run the build
export ARCH=x86_64
make -j4 --f Makefile

# rename the folder containing the binary we just compiled
if [ -f "./bin/libVVISF.a" ]; then
	echo "file exists!"
	mv "./bin" "./bin_x86_64"
else
	echo "file does not exist!"
	echo "ERROR: static library was not built or could not be found!"
	exit 1
fi

# clean whatever's there
make --f Makefile clean

# set the env var for the arm64 build, run the build
export ARCH=arm64
make -j4 --f Makefile

# rename the folder containing the binary we just compiled
if [ -f "./bin/libVVISF.a" ]; then
	echo "file exists!"
	mv "./bin" "./bin_arm64"
else
	echo "file does not exist!"
	echo "ERROR: static library was not built or could not be found!"
	exit 2
fi

# make a new binary folder
mkdir "./bin"

# run lipo, sticking the two binaries together to make a universal binary!
lipo -create "./bin_x86_64/libVVISF.a" "./bin_arm64/libVVISF.a" -output "./bin/libVVISF.a"

# clean up the platform-specific binary directories that won't be cleaned up by make
rm -rf "./bin_x86_64"
rm -rf "./bin_arm64"

echo "************** Successfully finished building universal binary!"
exit 0
