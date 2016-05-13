mkdir build-gcc
cd build-gcc
cmake .. -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE=MinSizeRel
mingw32-make
cd ..
pause