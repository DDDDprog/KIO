@echo off
echo Building KIO for Windows x64...
cmake -B build_win64 -G "MinGW Makefiles" -A x64
cmake --build build_win64 -j4
echo Build complete. Binaries are in build_win64/
pause
