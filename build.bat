@echo off
setlocal

echo Building CMake project...

if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

echo Configuring CMake...
cmake -DCMAKE_TOOLCHAIN_FILE="C:\Users\ichir\Desktop\cpp - test\vcpkg\scripts\buildsystems\vcpkg.cmake" ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo Building project...
cmake --build . --config Debug -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!