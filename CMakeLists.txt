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
#
#  Примеры:
#    cmake -S . -B build
#    cmake --build build --config Release
#
#    cmake -S . -B build -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_TESTS=ON
#    cmake --build build --config Release
#    ctest --test-dir build -C Release --output-on-failure
# ============================================================================
cmake_minimum_required(VERSION 3.16)

# Версия совпадает с BERESTA_VERSION_STRING в include/capi/beresta_c.h
project(beresta VERSION 2.1.0 LANGUAGES CXX)

# ---- Стандарт языка: строго C++17 без расширений компилятора ---------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BERESTA_BUILD_STATIC "Собирать статическую библиотеку для C++" OFF)
option(BERESTA_BUILD_EXAMPLES "Собирать демонстрационные программы" OFF)
option(BERESTA_BUILD_TESTS "Собирать модульные тесты" OFF)
option(BERESTA_BUILD_JAVA "Копировать бинарник в ресурсы Java-обёртки" OFF)
option(BERESTA_BUILD_CSHARP "Копировать бинарник в C#-обёртку" OFF)
option(BERESTA_BUILD_PYTHON "Копировать бинарник в Python-обёртку" OFF)
option(BERESTA_STATIC_RUNTIME "Встроить C++ runtime в динамическую библиотеку" ON)

# Тестам и демо на C++ нужна статическая библиотека: из динамической
# экспортируется только Си-интерфейс, классы C++ скрыты
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
function(beresta_apply_compile_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    endif()
endfunction()

# ---- Объектные файлы ядра: компилируются ОДИН раз для обеих библиотек --------
# PIC обязателен для .so; скрытая видимость оставляет в таблице экспорта
# только функции с BERESTA_API (Си-интерфейс).
file(GLOB_RECURSE BERESTA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE BERESTA_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_library(beresta_objects OBJECT ${BERESTA_SOURCES} ${BERESTA_HEADERS})
target_include_directories(beresta_objects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(beresta_objects PRIVATE BERESTA_BUILDING_SHARED)
set_target_properties(beresta_objects PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
beresta_apply_compile_options(beresta_objects)

# ---- Динамическая библиотека (цель по умолчанию) ------------------------------
# Имя файла одинаково для всех компиляторов одной ОС: beresta.dll (без
# префикса lib, который добавил бы MinGW), libberesta.so, libberesta.dylib.
add_library(beresta SHARED $<TARGET_OBJECTS:beresta_objects>)
add_library(beresta::beresta ALIAS beresta)
target_include_directories(beresta PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
# Потребители на Windows получают __declspec(dllimport) (BERESTA_USING_SHARED).
# INSTALL_NAME_DIR @rpath: на macOS программа ищет библиотеку рядом с собой
target_compile_definitions(beresta INTERFACE BERESTA_USING_SHARED)
set_target_properties(beresta PROPERTIES INSTALL_NAME_DIR "@rpath")
if(WIN32)
    set_target_properties(beresta PROPERTIES PREFIX "")
endif()
if(BERESTA_STATIC_RUNTIME AND MINGW)
    target_link_options(beresta PRIVATE -static)
elseif(BERESTA_STATIC_RUNTIME AND UNIX AND NOT APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(beresta PRIVATE -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL)
endif()

# ---- Статическая библиотека для программ на C++ --------------------------------
if(BERESTA_BUILD_STATIC)
    add_library(beresta_static STATIC $<TARGET_OBJECTS:beresta_objects>)
    add_library(beresta::static ALIAS beresta_static)
    target_include_directories(beresta_static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

# ---- Обёртки для других языков (по флагам) -------------------------------------
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

# ---- Демонстрационные программы ---------------------------------------------
if(BERESTA_BUILD_EXAMPLES)
    file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/src/*.cpp")
    add_executable(beresta_demo ${DEMO_SOURCES})
    target_include_directories(beresta_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/examples/include")
    target_link_libraries(beresta_demo PRIVATE beresta_static)
    beresta_apply_compile_options(beresta_demo)

    # Программа на чистом C через динамическую библиотеку: проверка того,
    # что beresta_c.h — корректный заголовок C99, а экспорт работает
    enable_language(C)
    add_executable(beresta_capi_demo "${CMAKE_CURRENT_SOURCE_DIR}/examples/capi/capi_demo.c")
    target_link_libraries(beresta_capi_demo PRIVATE beresta)
    set_target_properties(beresta_capi_demo PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
endif()

# ---- Модульные тесты (собственный мини-фреймворк, без внешних зависимостей) -
if(BERESTA_BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/src/*.cpp")
    add_executable(beresta_tests ${TEST_SOURCES})
    target_include_directories(beresta_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/include")
    target_link_libraries(beresta_tests PRIVATE beresta_static)
    beresta_apply_compile_options(beresta_tests)
    add_test(NAME beresta_tests COMMAND beresta_tests)
endif()
