# ============================================================================
#  Берёста — отладчик провалов W3C XML Schema Test Suite (CMake)
#  Компиляторы: MSVC, MinGW, GCC, Clang. ОС: Windows, Linux, macOS.
#
#  Отдельный проект поверх корневого: библиотека подключается через
#  add_subdirectory (статическая сборка), общий код прогонщика tests/xsts
#  (кроме его main.cpp) компилируется прямо в программу. Тип сборки по
#  умолчанию — Debug: точки останова работают и внутри библиотеки.
#
#    cmake -S tests/xsts_failing -B build-xsts-failing
#    cmake --build build-xsts-failing --config Debug
#    cmake --build build-xsts-failing --config Debug --target xsts_failing_run
#    cmake --build build-xsts-failing --config Debug --target xsts_failing_regen
#
#  Набор xsts-2007-06-20 должен лежать в tests/xsts/xmlschema2006-11-06
#  (make fetch в tests/xsts).
# ============================================================================
# Beresta — the W3C XML Schema Test Suite failure debugger (CMake)
# Compilers: MSVC, MinGW, GCC, Clang. OS: Windows, Linux, macOS.
# A separate project on top of the root one: the library is included through
# add_subdirectory (a static build), the shared code of the tests/xsts runner
# (except its main.cpp) is compiled right into the program. The default build
# type is Debug: breakpoints work inside the library too.
#   cmake -S tests/xsts_failing -B build-xsts-failing
#   cmake --build build-xsts-failing --config Debug
#   cmake --build build-xsts-failing --config Debug --target xsts_failing_run
#   cmake --build build-xsts-failing --config Debug --target xsts_failing_regen
# The xsts-2007-06-20 suite must be in tests/xsts/xmlschema2006-11-06
# (make fetch in tests/xsts).
# ============================================================================
cmake_minimum_required(VERSION 3.16)

# Debug по умолчанию — до project() и до корневого проекта (тот иначе выберет Release)
# Debug by default — before project() and the root project (it would pick Release otherwise)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Тип сборки / The build type" FORCE)
endif()

project(beresta_xsts_failing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Библиотека: корневой проект в режиме статической сборки ----------------
# ---- The library: the root project in static build mode ----------------
get_filename_component(BERESTA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(XSTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../xsts" ABSOLUTE)
set(BERESTA_BUILD_STATIC ON CACHE BOOL "Статическая библиотека для отладчика" FORCE)
add_subdirectory("${BERESTA_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/beresta")

# ---- Программа: свои исходники и прогонщик tests/xsts без его main.cpp -------
# ---- The program: its own sources and the tests/xsts runner without its main.cpp -------
file(GLOB_RECURSE FAILING_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE FAILING_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE XSTS_SOURCES CONFIGURE_DEPENDS "${XSTS_DIR}/src/*.cpp")
list(REMOVE_ITEM XSTS_SOURCES "${XSTS_DIR}/src/main.cpp")

add_executable(xsts_failing ${FAILING_SOURCES} ${FAILING_HEADERS} ${XSTS_SOURCES})
target_include_directories(xsts_failing PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" "${XSTS_DIR}/include")
target_link_libraries(xsts_failing PRIVATE beresta_static)
# Папка с failing_cases.tsv: IDE запускает программу из папки сборки
# The folder with failing_cases.tsv: an IDE starts the program from the build folder
target_compile_definitions(xsts_failing PRIVATE XSTS_FAILING_HOME="${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(xsts_failing PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
# Функция объявлена корневым CMakeLists.txt (функции CMake видны глобально)
# The function is declared by the root CMakeLists.txt (CMake functions are visible globally)
beresta_apply_compile_options(xsts_failing)

# ---- Прогон списка и его пересборка ------------------------------------------
# ---- Running the list and rebuilding it ------------------------------------------
add_custom_target(xsts_failing_run
    COMMAND xsts_failing
    DEPENDS xsts_failing
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Прогон провалов W3C XML Schema Test Suite"
    VERBATIM)
add_custom_target(xsts_failing_regen
    COMMAND xsts_failing --regenerate
    DEPENDS xsts_failing
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Пересборка failing_cases.tsv полным прогоном набора"
    VERBATIM)
