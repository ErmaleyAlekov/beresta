# ============================================================================
#  Берёста — замеры скорости и памяти против других XML-библиотек (CMake)
#  Компиляторы: MSVC, MinGW, GCC, Clang. ОС: Windows, Linux, macOS.
#
#  Отдельный проект поверх корневого: библиотека подключается через
#  add_subdirectory со статической сборкой. Сравниваемые библиотеки ищет
#  pkg-config (libxml2, pugixml, Xerces-C, tinyxml2), RapidXML — по
#  заголовку; ненайденные просто не участвуют. Отключить: -DBENCH_WITH_XERCES=OFF.
#    cmake -S benchmarks -B build-bench -DCMAKE_BUILD_TYPE=Release
#    cmake --build build-bench --config Release
#    cmake --build build-bench --config Release --target bench_run
# ============================================================================
#  Beresta — speed and memory measurements against other XML libraries (CMake)
#  Compilers: MSVC, MinGW, GCC, Clang. OS: Windows, Linux, macOS.
#
#  A separate project on top of the root one: the library is included through
#  add_subdirectory with a static build. The compared libraries are found by
#  pkg-config (libxml2, pugixml, Xerces-C, tinyxml2), RapidXML — by its header;
#  the ones not found simply do not take part. Disable: -DBENCH_WITH_XERCES=OFF.
# ============================================================================
cmake_minimum_required(VERSION 3.16)

project(beresta_benchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Библиотека: корневой проект в режиме статической сборки ----------------
# ---- The library: the root project in static build mode ----------------------
get_filename_component(BERESTA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(BERESTA_BUILD_STATIC ON CACHE BOOL "Статическая библиотека для замеров" FORCE)
add_subdirectory("${BERESTA_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/beresta")

# ---- Программа замеров / The benchmark program --------------------------------
file(GLOB_RECURSE BENCH_CORE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engines/beresta/*.cpp")
add_executable(beresta_bench "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" ${BENCH_CORE})
target_include_directories(beresta_bench PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(beresta_bench PRIVATE beresta_static)
if(WIN32)
    target_link_libraries(beresta_bench PRIVATE psapi)
endif()
# Функция объявлена корневым CMakeLists.txt / The function is declared by the root CMakeLists.txt
beresta_apply_compile_options(beresta_bench)

# ---- Сравниваемые библиотеки / The compared libraries ---------------------------
# Каждая найденная библиотека добавляет свои исходники, макрос и компоновку
# Each found library adds its sources, a macro and linking
find_package(PkgConfig QUIET)
function(bench_add_engine option package directory macro)
    option(${option} "Сравнивать с ${package} / Compare with ${package}" ON)
    if(NOT ${option} OR NOT PKG_CONFIG_FOUND)
        return()
    endif()
    pkg_check_modules(${macro}_PKG QUIET IMPORTED_TARGET ${package})
    if(NOT ${macro}_PKG_FOUND)
        return()
    endif()
    file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/src/engines/${directory}/*.cpp")
    target_sources(beresta_bench PRIVATE ${sources})
    target_compile_definitions(beresta_bench PRIVATE ${macro})
    target_link_libraries(beresta_bench PRIVATE PkgConfig::${macro}_PKG)
    message(STATUS "Берёста, замеры: подключена / benchmark: enabled ${package}")
endfunction()

bench_add_engine(BENCH_WITH_LIBXML2 libxml-2.0 libxml2 BENCH_WITH_LIBXML2)
bench_add_engine(BENCH_WITH_PUGIXML pugixml pugixml BENCH_WITH_PUGIXML)
bench_add_engine(BENCH_WITH_XERCES xerces-c xerces BENCH_WITH_XERCES)
bench_add_engine(BENCH_WITH_TINYXML2 tinyxml2 tinyxml2 BENCH_WITH_TINYXML2)

# RapidXML — только заголовки, pkg-config у него нет / RapidXML is headers only, it has no pkg-config
option(BENCH_WITH_RAPIDXML "Сравнивать с RapidXML / Compare with RapidXML" ON)
find_path(BENCH_RAPIDXML_INCLUDE rapidxml/rapidxml.hpp)
if(BENCH_WITH_RAPIDXML AND BENCH_RAPIDXML_INCLUDE)
    file(GLOB rapidxml_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/engines/rapidxml/*.cpp")
    target_sources(beresta_bench PRIVATE ${rapidxml_sources})
    target_include_directories(beresta_bench PRIVATE "${BENCH_RAPIDXML_INCLUDE}")
    target_compile_definitions(beresta_bench PRIVATE BENCH_WITH_RAPIDXML)
    message(STATUS "Берёста, замеры: подключена / benchmark: enabled RapidXML")
endif()

# ---- Замер с отчётом / The measurement with a report --------------------------
add_custom_target(bench_run
    COMMAND beresta_bench --report "${CMAKE_CURRENT_SOURCE_DIR}/benchmark_report.md"
    DEPENDS beresta_bench
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Берёста: замеры / Beresta: benchmarks"
    VERBATIM)
