# ============================================================================
#  Берёста — прогонщик W3C XML Conformance Test Suite (CMake)
#  Компиляторы: MSVC, MinGW, GCC, Clang. ОС: Windows, Linux, macOS.
#
#  Отдельный проект поверх корневого: библиотека подключается через
#  add_subdirectory со статической сборкой, прогонщик линкуется с ней.
#
#    cmake -S tests/conformance -B build-conformance
#    cmake --build build-conformance --config Release
#    cmake --build build-conformance --config Release --target conformance_run
#
#  Набор xmlts20130923 должен лежать в tests/conformance/xmlconf
#  (make fetch в этой папке или вручную: https://www.w3.org/XML/Test/).
# ============================================================================
# Beresta — the W3C XML Conformance Test Suite runner (CMake)
# Compilers: MSVC, MinGW, GCC, Clang. OS: Windows, Linux, macOS.
# A separate project on top of the root one: the library is included through
# add_subdirectory with a static build, the runner links with it.
#   cmake -S tests/conformance -B build-conformance
#   cmake --build build-conformance --config Release
#   cmake --build build-conformance --config Release --target conformance_run
# The xmlts20130923 suite must be in tests/conformance/xmlconf
# (make fetch in this folder or by hand: https://www.w3.org/XML/Test/).
# ============================================================================
cmake_minimum_required(VERSION 3.16)

project(beresta_conformance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Библиотека: корневой проект в режиме статической сборки ----------------
# ---- The library: the root project in static build mode ----------------
get_filename_component(BERESTA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(BERESTA_BUILD_STATIC ON CACHE BOOL "Статическая библиотека для прогонщика" FORCE)
add_subdirectory("${BERESTA_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/beresta")

# ---- Прогонщик ------------------------------------------------------------------
# ---- The runner ------------------------------------------------------------------
file(GLOB_RECURSE RUNNER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE RUNNER_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(conformance_runner ${RUNNER_SOURCES} ${RUNNER_HEADERS})
target_include_directories(conformance_runner PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(conformance_runner PRIVATE beresta_static)
# Функция объявлена корневым CMakeLists.txt (функции CMake видны глобально)
# The function is declared by the root CMakeLists.txt (CMake functions are visible globally)
beresta_apply_compile_options(conformance_runner)

# ---- Прогон всего набора с записью отчёта ------------------------------------------
# ---- Running the whole suite and writing the report ------------------------------------------
add_custom_target(conformance_run
    COMMAND conformance_runner "${CMAKE_CURRENT_SOURCE_DIR}/xmlconf"
            --report "${CMAKE_CURRENT_SOURCE_DIR}/conformance_report.md"
    DEPENDS conformance_runner
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Прогон W3C XML Conformance Test Suite"
    VERBATIM)
