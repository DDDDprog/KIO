@echo off
echo Building Axeon for Windows using Visual Studio...

REM Create build directory
if not exist build-vs mkdir build-vs
cd build-vs

REM Generate Visual Studio solution
echo Generating Visual Studio solution...
cmake .. -G "Visual Studio 17 2026" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DAXEON_BUILD_STATIC=ON ^
    -DAXEON_ENABLE_JIT=OFF ^
    -DAXEON_ENABLE_PARALLEL=OFF

REM Build with Visual Studio
echo Building with Visual Studio...
cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%

REM Copy the executable to the root directory
echo Copying executable...
copy Release\axeon.exe ..\axeon_vs.exe

REM Strip symbols (if strip is available)
where strip >nul 2>nul
if %errorlevel% equ 0 (
    echo Stripping symbols...
    strip ..\axeon_vs.exe
)

echo Build completed successfully!
echo Executable: axeon_vs.exe
echo You can now run: axeon_vs.exe ..\examples\hello.axe
cd ..
pause