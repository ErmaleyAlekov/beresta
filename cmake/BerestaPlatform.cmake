# ============================================================================
#  BerestaPlatform.cmake — определение целевой ОС и архитектуры процессора
#
#  Результат (переменные в области вызова):
#    BERESTA_PLATFORM_OS    windows | linux | macos | <имя системы>
#    BERESTA_PLATFORM_ARCH  x86_64 | aarch64 | x86 | <как сообщил компилятор>
#    BERESTA_PLATFORM       <os>-<arch>, например windows-x86_64
#                           (имя папки ресурсов Java и Python)
#    BERESTA_DOTNET_RID     идентификатор среды .NET: win-x64, linux-arm64,
#                           osx-arm64 ... (папка runtimes/<rid>/native NuGet)
#    BERESTA_PLATFORMS      список папок <os>-<arch>, куда кладётся бинарник;
#    BERESTA_DOTNET_RIDS    список RID. Обычно в них по одному значению, а
#                           универсальный бинарник macOS (CMAKE_OSX_ARCHITECTURES
#                           = "x86_64;arm64", BERESTA_PLATFORM_ARCH = universal)
#                           один и тот же файл кладётся в папки обеих архитектур
#
#  Важно: определяется ЦЕЛЕВАЯ платформа, а не та, на которой идёт сборка.
#  При кросс-компиляции (например, ARM64 на машине x64) бинарник должен
#  попасть в папку той архитектуры, где он будет работать.
# ============================================================================
# BerestaPlatform.cmake — detection of the target OS and processor architecture
# Result (variables in the calling scope):
#   BERESTA_PLATFORM_OS    windows | linux | macos | <system name>
#   BERESTA_PLATFORM_ARCH  x86_64 | aarch64 | x86 | <as the compiler reported>
#   BERESTA_PLATFORM       <os>-<arch>, for example windows-x86_64
#                          (the Java and Python resource folder name)
#   BERESTA_DOTNET_RID     the .NET runtime identifier: win-x64, linux-arm64,
#                          osx-arm64 ... (the NuGet runtimes/<rid>/native folder)
#   BERESTA_PLATFORMS      the list of <os>-<arch> folders the binary goes into;
#   BERESTA_DOTNET_RIDS    the list of RIDs. Usually they hold one value each, while
#                          for a universal macOS binary (CMAKE_OSX_ARCHITECTURES
#                          = "x86_64;arm64", BERESTA_PLATFORM_ARCH = universal)
#                          one and the same file goes into the folders of both architectures
# Important: the TARGET platform is detected, not the one the build runs on.
# With cross-compilation (for example, ARM64 on an x64 machine) the binary must
# go into the folder of the architecture where it will run.
# ============================================================================

# ---- Сырое имя архитектуры от того источника, который точнее всего ---------
# 1) MSVC сообщает архитектуру самого компилятора (x64, ARM64, X86) — это
#    верно и для генератора Visual Studio (-A ARM64), и для Ninja + vcvars;
# 2) на macOS явно заданная CMAKE_OSX_ARCHITECTURES важнее процессора хоста
#    (universal-сборку "arm64;x86_64" раскладывать по одной папке нельзя);
# 3) иначе — CMAKE_SYSTEM_PROCESSOR (при кросс-сборке его задаёт toolchain).
# ---- The raw architecture name from the most accurate source ---------
# 1) MSVC reports the architecture of the compiler itself (x64, ARM64, X86) — this
#    is correct both for the Visual Studio generator (-A ARM64) and for Ninja + vcvars;
# 2) on macOS an explicitly set CMAKE_OSX_ARCHITECTURES beats the host processor
#    (a universal build "arm64;x86_64" cannot be put into one folder);
# 3) otherwise — CMAKE_SYSTEM_PROCESSOR (in a cross build the toolchain sets it).
function(beresta_raw_architecture out_var)
    set(raw "${CMAKE_SYSTEM_PROCESSOR}")
    if(MSVC AND MSVC_CXX_ARCHITECTURE_ID)
        set(raw "${MSVC_CXX_ARCHITECTURE_ID}")
    elseif(APPLE AND CMAKE_OSX_ARCHITECTURES)
        # Несколько архитектур — универсальный (fat) бинарник: Apple Clang
        # компилирует каждую и склеивает результат сам, как lipo -create
        # Several architectures — a universal (fat) binary: Apple Clang
        # compiles each one and glues the result itself, like lipo -create
        list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
        set(raw "${CMAKE_OSX_ARCHITECTURES}")
        if(arch_count GREATER 1)
            set(raw "universal")
        endif()
    endif()
    string(TOLOWER "${raw}" raw)
    set(${out_var} "${raw}" PARENT_SCOPE)
endfunction()

# ---- Приведение к единым именам (те же имена использует загрузчик Java) -----
# Разные системы называют одну архитектуру по-разному: AMD64 (Windows),
# x86_64 (Linux, macOS), x64 (MSVC); arm64 (macOS, Windows) и aarch64 (Linux).
# ---- Normalizing to common names (the Java loader uses the same names) -----
# Different systems name the same architecture differently: AMD64 (Windows),
# x86_64 (Linux, macOS), x64 (MSVC); arm64 (macOS, Windows) and aarch64 (Linux).
function(beresta_normalize_architecture raw out_var)
    set(arch "${raw}")
    if(raw MATCHES "^(x86_64|amd64|x64|em64t)$")
        set(arch "x86_64")
    elseif(raw MATCHES "^(aarch64|arm64|armv8.*)$")
        set(arch "aarch64")
    elseif(raw MATCHES "^(x86|i[3-6]86|win32)$")
        set(arch "x86")
    endif()
    # 32-битный компилятор на 64-битном процессоре собирает 32-битный код
    # A 32-bit compiler on a 64-bit processor builds 32-bit code
    if(arch STREQUAL "x86_64" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(arch "x86")
    endif()
    set(${out_var} "${arch}" PARENT_SCOPE)
endfunction()

# ---- Идентификатор среды .NET (RID) по ОС и архитектуре ----------------------
# ---- The .NET runtime identifier (RID) by OS and architecture ----------------------
function(beresta_dotnet_rid os arch out_var)
    set(rid_os "${os}")
    if(os STREQUAL "windows")
        set(rid_os "win")
    elseif(os STREQUAL "macos")
        set(rid_os "osx")
    endif()
    set(rid_arch "${arch}")
    if(arch STREQUAL "x86_64")
        set(rid_arch "x64")
    elseif(arch STREQUAL "aarch64")
        set(rid_arch "arm64")
    endif()
    set(${out_var} "${rid_os}-${rid_arch}" PARENT_SCOPE)
endfunction()

# ---- Главная функция модуля --------------------------------------------------
# Значения можно переопределить из командной строки:
#   cmake -DBERESTA_PLATFORM_ARCH=aarch64 ...
# ---- The main function of the module --------------------------------------------------
# The values can be overridden from the command line:
#   cmake -DBERESTA_PLATFORM_ARCH=aarch64 ...
function(beresta_detect_platform)
    if(WIN32)
        set(os "windows")
    elseif(APPLE)
        set(os "macos")
    else()
        string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
    endif()
    beresta_raw_architecture(raw_arch)
    beresta_normalize_architecture("${raw_arch}" arch)
    set(BERESTA_PLATFORM_OS "${os}" CACHE STRING "Целевая ОС для папок ресурсов")
    set(BERESTA_PLATFORM_ARCH "${arch}" CACHE STRING "Целевая архитектура для папок ресурсов")
    set(BERESTA_PLATFORM "${BERESTA_PLATFORM_OS}-${BERESTA_PLATFORM_ARCH}" PARENT_SCOPE)
    beresta_dotnet_rid("${BERESTA_PLATFORM_OS}" "${BERESTA_PLATFORM_ARCH}" rid)
    set(BERESTA_DOTNET_RID "${rid}" PARENT_SCOPE)
    set(platforms "${BERESTA_PLATFORM_OS}-${BERESTA_PLATFORM_ARCH}")
    set(rids "${rid}")
    if(BERESTA_PLATFORM_ARCH STREQUAL "universal")
        set(platforms "${BERESTA_PLATFORM_OS}-x86_64" "${BERESTA_PLATFORM_OS}-aarch64")
        set(rids "osx-x64" "osx-arm64")
    endif()
    set(BERESTA_PLATFORMS "${platforms}" PARENT_SCOPE)
    set(BERESTA_DOTNET_RIDS "${rids}" PARENT_SCOPE)
endfunction()
