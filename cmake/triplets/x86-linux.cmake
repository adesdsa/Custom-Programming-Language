# picoC overlay triplet for 32-bit Linux (x86)
# Use: cmake -DCMAKE_TOOLCHAIN_FILE=... -DVCPKG_TARGET_TRIPLET=x86-linux ...
# Requires: g++-multilib, libc6-dev-i386
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
