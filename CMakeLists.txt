# ============================================================================
#  Берёста (beresta) — кроссплатформенная сборка CMake
#  Компиляторы: MSVC, MinGW, GCC, Clang. ОС: Windows, Linux, macOS.
#
#  По умолчанию собирается ТОЛЬКО ядро — динамическая библиотека
#  (beresta.dll / libberesta.so / libberesta.dylib) с плоским Си-интерфейсом.
#  Всё остальное включается флагами:
#
#    -DBERESTA_BUILD_STATIC=ON    статическая библиотека для C++ (beresta_static)
#    -DBERESTA_BUILD_EXAMPLES=ON  демо-программы на C++ и на C
#    -DBERESTA_BUILD_TESTS=ON     модульные тесты (и самопроверки обёрток)
#    -DBERESTA_BUILD_JAVA=ON      бинарник -> ресурсы Maven-проекта (Panama)
#    -DBERESTA_BUILD_CSHARP=ON    бинарник -> runtimes/<rid>/native (P/Invoke)
#    -DBERESTA_BUILD_PYTHON=ON    бинарник -> пакет Python (ctypes)
#    -DBERESTA_BUILD_PHP=ON       бинарник -> пакет Composer (PHP FFI)
#    -DBERESTA_BUILD_NODE=ON     аддон beresta.node -> пакет Node.js (Node-API)
#    -DBERESTA_ERRORS_LANG=en     язык сообщений об ошибках (по умолчанию ru)
#                                 error message language (default ru)
#
#  Примеры:
#    cmake -S . -B build
#    cmake --build build --config Release
#
#    cmake -S . -B build -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_TESTS=ON
#    cmake --build build --config Release
#    ctest --test-dir build -C Release --output-on-failure
#
#    # macOS: один универсальный бинарник для Intel и Apple Silicon
#    cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
#
#    # WebAssembly (браузер, Workers): см. cmake/BerestaWasm.cmake
#    emcmake cmake -S . -B build-wasm
# ============================================================================
# Beresta — cross-platform CMake build
# Compilers: MSVC, MinGW, GCC, Clang. OS: Windows, Linux, macOS.
# By default ONLY the core is built — the shared library
# (beresta.dll / libberesta.so / libberesta.dylib) with a flat C interface.
# Everything else is enabled with flags:
#   -DBERESTA_BUILD_STATIC=ON    a static library for C++ (beresta_static)
#   -DBERESTA_BUILD_EXAMPLES=ON  demo programs in C++ and C
#   -DBERESTA_BUILD_TESTS=ON     unit tests (and wrapper self-tests)
#   -DBERESTA_BUILD_JAVA=ON      binary -> Maven project resources (Panama)
#   -DBERESTA_BUILD_CSHARP=ON    binary -> runtimes/<rid>/native (P/Invoke)
#   -DBERESTA_BUILD_PYTHON=ON    binary -> the Python package (ctypes)
#   -DBERESTA_BUILD_PHP=ON       binary -> the Composer package (PHP FFI)
#   -DBERESTA_BUILD_NODE=ON     the beresta.node addon -> the Node.js package (Node-API)
#   -DBERESTA_ERRORS_LANG=en     the error message language (default ru)
# Examples: see above. On macOS -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" builds one
# universal binary for Intel and Apple Silicon.
# WebAssembly: emcmake cmake -S . -B build-wasm (see cmake/BerestaWasm.cmake).
# ============================================================================
cmake_minimum_required(VERSION 3.16)

# Минимальная версия macOS — 11.0, первая с Apple Silicon. Задаётся до project():
# так её подхватывают все цели. На других ОС переменная ни на что не влияет.
# The minimum macOS version is 11.0, the first one with Apple Silicon. It is set before project():
# this way all targets pick it up. On other OSes the variable has no effect.
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Минимальная версия macOS / The minimum macOS version")

# Версия совпадает с BERESTA_VERSION_STRING в include/capi/beresta_c.h
# The version matches BERESTA_VERSION_STRING in include/capi/beresta_c.h
project(beresta VERSION 2.1.4 LANGUAGES CXX)

# ---- Тип сборки по умолчанию — Release ---------------------------------------
# Без него однотипные генераторы (Make, Ninja) собирают без оптимизаций: ядро
# в разы медленнее, а модуль WebAssembly — в 13 раз медленнее и в 10 раз
# больше. Многоконфигурационные (Visual Studio, Xcode) выбирают тип сами.
# ---- The default build type is Release ---------------------------------------
# Without it single-config generators (Make, Ninja) build without optimizations: the core
# is several times slower, and the WebAssembly module is 13 times slower and 10 times
# larger. Multi-config generators (Visual Studio, Xcode) choose the type themselves.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Тип сборки / The build type" FORCE)
endif()

# ---- Стандарт языка: строго C++17 без расширений компилятора ---------------
# ---- Language standard: strictly C++17 without compiler extensions ---------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BERESTA_BUILD_STATIC "Собирать статическую библиотеку для C++" OFF)
option(BERESTA_BUILD_EXAMPLES "Собирать демонстрационные программы" OFF)
option(BERESTA_BUILD_TESTS "Собирать модульные тесты" OFF)
option(BERESTA_BUILD_JAVA "Копировать бинарник в ресурсы Java-обёртки" OFF)
option(BERESTA_BUILD_CSHARP "Копировать бинарник в C#-обёртку" OFF)
option(BERESTA_BUILD_PYTHON "Копировать бинарник в Python-обёртку" OFF)
option(BERESTA_BUILD_PHP "Копировать бинарник в PHP-обёртку / Copy the binary into the PHP binding" OFF)
option(BERESTA_BUILD_NODE "Собирать аддон Node.js / Build the Node.js addon" OFF)
option(BERESTA_STATIC_RUNTIME "Встроить C++ runtime в динамическую библиотеку" ON)

# ---- Язык сообщений об ошибках / Error message language ----------------------
# Список языков берётся из имён каталогов src/i18n/catalogs/messages_<код>.cpp:
# новый каталог сразу появляется среди вариантов (cmake-gui, ccmake). Неизвестный
# код останавливает настройку, а не превращается в молчаливый английский.
# The language list comes from the catalog names src/i18n/catalogs/messages_<code>.cpp:
# a new catalog appears among the choices at once (cmake-gui, ccmake). An unknown
# code stops configuration instead of silently becoming English.
file(GLOB BERESTA_CATALOG_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/catalogs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/catalogs/messages_*.cpp")
string(REGEX REPLACE "messages_([^;]+)\\.cpp" "\\1" BERESTA_LANGUAGES "${BERESTA_CATALOG_FILES}")
set(BERESTA_ERRORS_LANG "ru" CACHE STRING "Язык сообщений об ошибках / Error message language")
set_property(CACHE BERESTA_ERRORS_LANG PROPERTY STRINGS ${BERESTA_LANGUAGES})
if(NOT BERESTA_ERRORS_LANG IN_LIST BERESTA_LANGUAGES)
    message(FATAL_ERROR "Берёста: нет каталога сообщений для языка / no message catalog for language "
        "\"${BERESTA_ERRORS_LANG}\" (${BERESTA_LANGUAGES})")
endif()

# Тестам и демо на C++ нужна статическая библиотека: из динамической
# экспортируется только Си-интерфейс, классы C++ скрыты
# Tests and C++ demos need the static library: the shared one exports
# only the C interface, the C++ classes are hidden
if(BERESTA_BUILD_TESTS OR BERESTA_BUILD_EXAMPLES)
    set(BERESTA_BUILD_STATIC ON)
endif()

# ---- Независимость бинарника от C++ runtime ----------------------------------
# Jar, NuGet-пакет и Python-пакет переносят ОДИН файл библиотеки. Если она
# зависит от libstdc++-6.dll / vcruntime140.dll, на чистой машине загрузка
# упадёт. Поэтому по умолчанию runtime встраивается в саму библиотеку:
# наружу из неё не выходят ни исключения, ни объекты STL — это безопасно.
# На Linux --exclude-libs,ALL прячет символы встроенной libstdc++, чтобы
# они не конфликтовали с системной libstdc++ в процессе JVM или Python.
# ---- Independence of the binary from the C++ runtime ----------------------------------
# The jar, the NuGet package and the Python package carry ONE library file. If it
# depends on libstdc++-6.dll / vcruntime140.dll, loading fails on a clean machine.
# That is why by default the runtime is embedded into the library itself:
# neither exceptions nor STL objects leave it — this is safe.
# On Linux --exclude-libs,ALL hides the symbols of the embedded libstdc++ so that
# they do not conflict with the system libstdc++ in a JVM or Python process.
if(BERESTA_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BerestaPlatform.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BerestaBindings.cmake")
beresta_detect_platform()
message(STATUS "Берёста: целевая платформа ${BERESTA_PLATFORM} (.NET RID ${BERESTA_DOTNET_RID})")

# ---- Единые флаги предупреждений и кодировки для всех целей -----------------
# /utf-8 для MSVC обязателен: исходники и строковые литералы в UTF-8
# (кириллица в сообщениях об ошибках), иначе MSVC читает их в кодировке ANSI.
# ---- Common warning and encoding flags for all targets -----------------
# /utf-8 for MSVC is mandatory: sources and string literals are in UTF-8
# (Cyrillic in error messages), otherwise MSVC reads them in the ANSI encoding.
function(beresta_apply_compile_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    endif()
endfunction()

# ---- Параметры компоновки модулей, которые уезжают в пакеты обёрток ----------
# Встроенный C++ runtime (см. выше) и, кроме отладочной сборки, удаление
# отладочных символов (-s): без него beresta.dll от MinGW весит ~18 МБ вместо
# ~2 МБ. MSVC символы в DLL не кладёт (они в .pdb), у Apple ld ключ -s устарел.
# ---- Link options of the modules that travel in the binding packages ----------
# The embedded C++ runtime (see above) and, except for a debug build, stripping
# the debug symbols (-s): without it the MinGW beresta.dll weighs ~18 MB instead
# of ~2 MB. MSVC does not put symbols into a DLL (they go to .pdb), Apple ld's -s is obsolete.
function(beresta_apply_link_options target)
    if(BERESTA_STATIC_RUNTIME AND MINGW)
        target_link_options(${target} PRIVATE -static)
    elseif(BERESTA_STATIC_RUNTIME AND UNIX AND NOT APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(${target} PRIVATE -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL)
    endif()
    if(NOT MSVC AND NOT APPLE)
        target_link_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:-s>)
    endif()
endfunction()

# ---- std::filesystem на GCC 8 -------------------------------------------------
# В libstdc++ до GCC 9 реализация std::filesystem лежит в отдельной статической
# библиотеке libstdc++fs.a (так в Astra Linux SE 1.7, Debian 10). Её получают
# все модули с объектами ядра; программы — через beresta_static (PUBLIC).
# ---- std::filesystem on GCC 8 -------------------------------------------------
# In libstdc++ before GCC 9 the std::filesystem implementation lives in a separate
# static library libstdc++fs.a (as in Astra Linux SE 1.7, Debian 10). It is given to
# all modules with the core objects; programs get it through beresta_static (PUBLIC).
set(BERESTA_FS_LIBS "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set(BERESTA_FS_LIBS stdc++fs)
endif()

# ---- Объектные файлы ядра: компилируются ОДИН раз для обеих библиотек --------
# PIC обязателен для .so; скрытая видимость оставляет в таблице экспорта
# только функции с BERESTA_API (Си-интерфейс).
# ---- Core object files: compiled ONCE for both libraries --------
# PIC is mandatory for .so; hidden visibility leaves in the export table
# only the functions with BERESTA_API (the C interface).
file(GLOB_RECURSE BERESTA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE BERESTA_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_library(beresta_objects OBJECT ${BERESTA_SOURCES} ${BERESTA_HEADERS})
target_include_directories(beresta_objects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(beresta_objects PRIVATE BERESTA_BUILDING_SHARED
    BERESTA_ERRORS_LANG=${BERESTA_ERRORS_LANG})
set_target_properties(beresta_objects PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
beresta_apply_compile_options(beresta_objects)

# ---- WebAssembly: под Emscripten собирается только модуль Wasm -----------------
# ---- WebAssembly: under Emscripten only the Wasm module is built -----------------
if(EMSCRIPTEN)
    if(BERESTA_BUILD_TESTS)
        enable_testing()
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BerestaWasm.cmake")
    beresta_setup_wasm()
    return()
endif()

# ---- Динамическая библиотека (цель по умолчанию) ------------------------------
# Имя файла одинаково для всех компиляторов одной ОС: beresta.dll (без
# префикса lib, который добавил бы MinGW), libberesta.so, libberesta.dylib.
# ---- The shared library (the default target) ------------------------------
# The file name is the same for all compilers of one OS: beresta.dll (without
# the lib prefix that MinGW would add), libberesta.so, libberesta.dylib.
add_library(beresta SHARED $<TARGET_OBJECTS:beresta_objects>)
add_library(beresta::beresta ALIAS beresta)
target_include_directories(beresta PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
# Потребители на Windows получают __declspec(dllimport) (BERESTA_USING_SHARED).
# INSTALL_NAME_DIR @rpath: на macOS программа ищет библиотеку рядом с собой
# Consumers on Windows get __declspec(dllimport) (BERESTA_USING_SHARED).
# INSTALL_NAME_DIR @rpath: on macOS a program looks for the library next to itself
target_compile_definitions(beresta INTERFACE BERESTA_USING_SHARED)
set_target_properties(beresta PROPERTIES INSTALL_NAME_DIR "@rpath")
if(WIN32)
    set_target_properties(beresta PROPERTIES PREFIX "")
endif()
beresta_apply_link_options(beresta)
target_link_libraries(beresta PRIVATE ${BERESTA_FS_LIBS})

# ---- Статическая библиотека для программ на C++ --------------------------------
# ---- The static library for C++ programs --------------------------------
if(BERESTA_BUILD_STATIC)
    add_library(beresta_static STATIC $<TARGET_OBJECTS:beresta_objects>)
    add_library(beresta::static ALIAS beresta_static)
    target_include_directories(beresta_static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(beresta_static PUBLIC ${BERESTA_FS_LIBS})
endif()

# ---- Обёртки для других языков (по флагам) -------------------------------------
# ---- Wrappers for other languages (by flags) -------------------------------------
if(BERESTA_BUILD_TESTS)
    enable_testing()
endif()
if(BERESTA_BUILD_JAVA)
    beresta_setup_java()
endif()
if(BERESTA_BUILD_CSHARP)
    beresta_setup_csharp()
endif()
if(BERESTA_BUILD_PYTHON)
    beresta_setup_python()
endif()
if(BERESTA_BUILD_PHP)
    beresta_setup_php()
endif()
if(BERESTA_BUILD_NODE)
    beresta_setup_node()
endif()

# ---- Демонстрационные программы ---------------------------------------------
# ---- Demo programs ---------------------------------------------
if(BERESTA_BUILD_EXAMPLES)
    file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/src/*.cpp")
    add_executable(beresta_demo ${DEMO_SOURCES})
    target_include_directories(beresta_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/examples/include")
    target_link_libraries(beresta_demo PRIVATE beresta_static)
    beresta_apply_compile_options(beresta_demo)

    # Программа на чистом C через динамическую библиотеку: проверка того,
    # что beresta_c.h — корректный заголовок C99, а экспорт работает
    # A program in pure C through the shared library: checks that
    # beresta_c.h is a correct C99 header and that the export works
    enable_language(C)
    add_executable(beresta_capi_demo "${CMAKE_CURRENT_SOURCE_DIR}/examples/capi/capi_demo.c")
    target_link_libraries(beresta_capi_demo PRIVATE beresta)
    set_target_properties(beresta_capi_demo PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
endif()

# ---- Модульные тесты (собственный мини-фреймворк, без внешних зависимостей) -
# ---- Unit tests (an own mini framework, without external dependencies) -
if(BERESTA_BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/src/*.cpp")
    add_executable(beresta_tests ${TEST_SOURCES})
    target_include_directories(beresta_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/include")
    target_link_libraries(beresta_tests PRIVATE beresta_static)

    # Тесты запускают std::thread: до glibc 2.34 (Astra Linux SE 1.7) нужна
    # отдельная libpthread, Threads::Threads подключает её там, где она нужна
    # The tests start std::thread: before glibc 2.34 (Astra Linux SE 1.7) a separate
    # libpthread is needed, Threads::Threads links it where it is needed
    find_package(Threads REQUIRED)
    target_link_libraries(beresta_tests PRIVATE Threads::Threads)
    beresta_apply_compile_options(beresta_tests)
    add_test(NAME beresta_tests COMMAND beresta_tests)
endif()
