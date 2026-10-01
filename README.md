# webrtc-chat
### Windows 11 Setup Guide

- Install Visual Studio 2022 with the C++ Development option, with Linux support selected (if you want to build for Linux using wsl2).

- Open VS 2022 Developer PowerShell (this is needed as env variables needed for cl.exe and other tools have been automatically set). You could also use an environment that sets up env variables needed for MSVC, e.g., selecting Visual Studio Toolchain in CLion, and running a CMake Preset through the GUI to
  generate necessary build files.

```
git clone --recurse-submodules https://github.com/mem64-bits/OpenGL-CMake-Template

cd vcpkg
# Setups vcpkg executable for project (only needed once)
.\bootstrap-vcpkg.bat

#Select either "windows-msvc-debug" or "windows-msvc-release"
cmake --preset <windows-msvc-debug / windows-msvc-release>

# After the cmake build files have been generated and vcpkg packages
# have been installed the project can be built through the IDE
# "VS # 2022" or "Clion"

# To manually build run the preset "windows-msvc-debug" for the debug build
cmake --build out/build/-windows-msvc-debug

# And run "windows-msvc-release" for the release build
cmake --build out/build/windows-msvc-release

Finally run the resulting executable in the out/build/linux-g++-debug/
or out/build/windows-msvc-release directory.

# For example:

# For Release builds:
cd out/build/windows-msvc-release 
.\OpenGL-CMake-Template.exe

# For Debug builds:
cd out/build/windows-msvc-debug 
.\OpenGL-CMake-Template.exe
```

## Install Guide for Linux

### Ubuntu / Debian

Ubuntu has the greatest support for vcpkg. Install any packages that vcpkg will ask for if needed.

```
# Ensure cmake and build tools are installed
sudo apt install cmake ninja-build build-essential
```

```
git clone --recurse-submodules https://github.com/mem64-bits/OpenGL-CMake-Template

cd OpenGL-CMake-Template/vcpkg
# Setups vcpkg executable for project (only needed once)
./bootstrap-vcpkg.sh

# Go back to the project root
cd ..

#Select either "linux-g++-debug" or "linux-g++-release"
cmake --preset <linux-g++-debug / windows-msvc-linux-g++-release>

# After the cmake build files have been generated and vcpkg packages
# have been installed the project can be built manually or through
the IDE "Clion".

# To manually build run the preset "windows-msvc-debug" for the debug build
cmake --build out/build/linux-g++-debug/

# And run "windows-msvc-release" for the release build
cmake --build out/build/linux-g++-release/

Finally run the resulting executable in the out/build/linux-g++-debug/ for
the debug build, or out/build/linux-g++-release/ for the release build.

# For example:

# For Release builds:
cd out/build/linux-g++-release 
./OpenGL-CMake-Template

# For Debug builds:
cd out/build/linux-g++-debug
./OpenGL-CMake-Template
```

### Fedora

Install a desktop environment with x11 / wayland support and
install these packages to ensure the buildsystem works

```
sudo dnf group install "development-tools" ninja-build cmake

```

```
git clone --recurse-submodules https://github.com/mem64-bits/OpenGL-CMake-Template

cd OpenGL-CMake-Template/vcpkg
# Setups vcpkg executable for project (only needed once)
./bootstrap-vcpkg.sh

# Go back to the project root
cd ..

#Select either "linux-g++-debug" or "linux-g++-release"
cmake --preset <linux-g++-debug / windows-msvc-linux-g++-release>

# After the cmake build files have been generated and vcpkg packages
# have been installed the project can be built manually or through
the IDE "Clion".

# To manually build run the preset "windows-msvc-debug" for the debug build
cmake --build out/build/linux-g++-debug/

# And run "windows-msvc-release" for the release build
cmake --build out/build/linux-g++-release/

Finally run the resulting executable in the out/build/linux-g++-debug/ for
the debug build, or out/build/linux-g++-release/ for the release build.

# For example:

# For Release builds:
cd out/build/linux-g++-release 
./OpenGL-CMake-Template

# For Debug builds:
cd out/build/linux-g++-debug
./OpenGL-CMake-Template
```

### Arch

Install a desktop environment with x11 / wayland support and
install these packages to ensure the buildsystem works

```
sudo pacman -Syu --needed base-devel cmake ninja
```

```
git clone --recurse-submodules https://github.com/mem64-bits/OpenGL-CMake-Template

cd OpenGL-CMake-Template/vcpkg
# Setups vcpkg executable for project (only needed once)
./bootstrap-vcpkg.sh

# Go back to the project root
cd ..

#Select either "linux-g++-debug" or "linux-g++-release"
cmake --preset <linux-g++-debug / windows-msvc-linux-g++-release>

# After the cmake build files have been generated and vcpkg packages
# have been installed the project can be built manually or through
the IDE "Clion".

# To manually build run the preset "windows-msvc-debug" for the debug build
cmake --build out/build/linux-g++-debug/

# And run "windows-msvc-release" for the release build
cmake --build out/build/linux-g++-release/

Finally run the resulting executable in the out/build/linux-g++-debug/ for
the debug build, or out/build/linux-g++-release/ for the release build.

# For example:

# For Release builds:
cd out/build/linux-g++-release 
./OpenGL-CMake-Template

# For Debug builds:
cd out/build/linux-g++-debug
./OpenGL-CMake-Template
```
