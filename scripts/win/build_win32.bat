@echo off
echo Building KIO for Windows x32...
cmake -B build_win32 -G "MinGW Makefiles" -A Win32
cmake --build build_win32 -j4
echo Build complete. Binaries are in build_win32/
pause
