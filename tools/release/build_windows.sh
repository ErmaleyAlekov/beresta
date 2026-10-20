#!/usr/bin/env bash
# ============================================================================
#  Сборка SDK для Windows x86_64 компилятором MinGW-w64 при выпуске на Gitverse
#
#  Использование (из корня исходного текста): build_windows.sh <тег vX.Y.Z> <каталог>
#  Раннер Gitverse — Linux, поэтому Windows собирается кросс-компилятором через
#  CMake. Архив — beresta-<версия>-windows-x86_64-mingw.zip:
#    beresta.dll        — Си-интерфейс, C++ runtime встроен (нужны только системные DLL);
#    libberesta.dll.a   — библиотека импорта для MinGW;
#    beresta.def        — список экспорта; для MSVC: lib /def:beresta.def /machine:x64;
#    libberesta_static.a — статическая библиотека C++ для MinGW.
#  Тесты и демо собираются (проверка компоновки), но не запускаются: на раннере нет Windows.
# ============================================================================
#  Building the SDK for Windows x86_64 with the MinGW-w64 compiler for a release on Gitverse
#
#  Usage (from the source root): build_windows.sh <tag vX.Y.Z> <directory>
#  The Gitverse runner is Linux, so Windows is built by a cross compiler through
#  CMake. The archive is beresta-<version>-windows-x86_64-mingw.zip:
#    beresta.dll        — the C interface, the C++ runtime is embedded (only system DLLs are needed);
#    libberesta.dll.a   — the import library for MinGW;
#    beresta.def        — the export list; for MSVC: lib /def:beresta.def /machine:x64;
#    libberesta_static.a — the C++ static library for MinGW.
#  The tests and demos are built (a link check) but not run: the runner has no Windows.
# ============================================================================
set -euo pipefail

. "$(dirname "$0")/lib/build_common.sh"

# Каталог сборки CMake / The CMake build directory
BUILD_DIR="build-mingw"

# Ставит MinGW-w64 (вариант posix: нужен std::thread) и CMake
# Installs MinGW-w64 (the posix flavor: std::thread is needed) and CMake
windows_tools() {
    apt_install g++-mingw-w64-x86-64-posix gcc-mingw-w64-x86-64-posix cmake make zip
    x86_64-w64-mingw32-g++-posix --version | head -n 1
}

# Кросс-сборка CMake: -static у программ — тесты и демо запускаются на любой Windows;
# --output-def пишет список экспорта DLL рядом с ней
# The CMake cross build: -static for programs — the tests and demos run on any Windows;
# --output-def writes the DLL export list next to it
build_windows() {
    cmake -S . -B "$BUILD_DIR" -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_SYSTEM_PROCESSOR=AMD64 \
        -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix \
        -DCMAKE_BUILD_TYPE=Release -DBERESTA_BUILD_STATIC=ON -DBERESTA_BUILD_TESTS=ON \
        -DBERESTA_BUILD_EXAMPLES=ON -DCMAKE_EXE_LINKER_FLAGS=-static \
        "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,--output-def,beresta.def"
    cmake --build "$BUILD_DIR" -j"$(build_jobs)"
}

# Точка входа: тег vX.Y.Z превращается в версию X.Y.Z
# The entry point: the vX.Y.Z tag turns into the X.Y.Z version
main() {
    local version="${1#v}" out="$2"
    windows_tools
    build_windows
    sdk_zip "$version" windows-x86_64-mingw "$out" "${BUILD_DIR}/beresta.dll" \
        "${BUILD_DIR}/libberesta.dll.a" "${BUILD_DIR}/beresta.def" "${BUILD_DIR}/libberesta_static.a"
}

main "$@"
