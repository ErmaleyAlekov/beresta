# ============================================================================
#  Берёста — прогонщик W3C XML Schema Test Suite (XSTS) (CMake)
#  Компиляторы: MSVC, MinGW, GCC, Clang. ОС: Windows, Linux, macOS.
#
#  Отдельный проект поверх корневого: библиотека подключается через
#  add_subdirectory со статической сборкой, прогонщик линкуется с ней.
#
#    cmake -S tests/xsts -B build-xsts
#    cmake --build build-xsts --config Release
#    cmake --build build-xsts --config Release --target xsts_run
#
#  Набор xsts-2007-06-20 должен лежать в tests/xsts/xmlschema2006-11-06
#  (make fetch в этой папке или вручную: https://www.w3.org/XML/2004/xml-schema-test-suite/).
# ============================================================================
# Beresta — the W3C XML Schema Test Suite (XSTS) runner (CMake)
# Compilers: MSVC, MinGW, GCC, Clang. OS: Windows, Linux, macOS.
# A separate project on top of the root one: the library is included through
# add_subdirectory with a static build, the runner links with it.
#   cmake -S tests/xsts -B build-xsts
#   cmake --build build-xsts --config Release
#   cmake --build build-xsts --config Release --target xsts_run
# The xsts-2007-06-20 suite must be in tests/xsts/xmlschema2006-11-06
# (make fetch in this folder or by hand: https://www.w3.org/XML/2004/xml-schema-test-suite/).
# ============================================================================
cmake_minimum_required(VERSION 3.16)

project(beresta_xsts LANGUAGES CXX)

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

add_executable(xsts_runner ${RUNNER_SOURCES} ${RUNNER_HEADERS})
target_include_directories(xsts_runner PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(xsts_runner PRIVATE beresta_static)
# Функция объявлена корневым CMakeLists.txt (функции CMake видны глобально)
# The function is declared by the root CMakeLists.txt (CMake functions are visible globally)
beresta_apply_compile_options(xsts_runner)

# ---- Прогон всего набора с записью отчёта ------------------------------------------
# ---- Running the whole suite and writing the report ------------------------------------------
add_custom_target(xsts_run
    COMMAND xsts_runner "${CMAKE_CURRENT_SOURCE_DIR}/xmlschema2006-11-06"
            --limitations "${CMAKE_CURRENT_SOURCE_DIR}/known_limitations.txt"
            --report "${CMAKE_CURRENT_SOURCE_DIR}/xsts_report.md"
    DEPENDS xsts_runner
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Прогон W3C XML Schema Test Suite"
    VERBATIM)
