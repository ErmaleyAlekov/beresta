# ============================================================================
#  Берёста — цели libFuzzer (CMake). Только Clang: -fsanitize=fuzzer есть
#  лишь в нём (Linux, macOS; на Windows — clang-cl с libFuzzer из LLVM).
#
#    CXX=clang++ cmake -S tests/fuzz -B build-fuzz -DCMAKE_BUILD_TYPE=RelWithDebInfo
#    cmake --build build-fuzz -j
#    build-fuzz/fuzz_parser -max_total_time=600 -dict=tests/fuzz/dict/xml.dict corpus/parser
#
#  Библиотека компилируется заново с инструментированием покрытия и
#  санитайзерами ASan + UBSan, чтобы находки были и в коде библиотеки.
# ============================================================================
# Beresta — libFuzzer targets (CMake). Clang only: only it has
# -fsanitize=fuzzer (Linux, macOS; on Windows — clang-cl with libFuzzer from LLVM).
#   CXX=clang++ cmake -S tests/fuzz -B build-fuzz -DCMAKE_BUILD_TYPE=RelWithDebInfo
#   cmake --build build-fuzz -j
#   build-fuzz/fuzz_parser -max_total_time=600 -dict=tests/fuzz/dict/xml.dict corpus/parser
# The library is compiled anew with coverage instrumentation and the
# ASan + UBSan sanitizers so that findings are in the library code too.
# ============================================================================
cmake_minimum_required(VERSION 3.16)

project(beresta_fuzz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Цели libFuzzer собираются только Clang (сейчас: ${CMAKE_CXX_COMPILER_ID})")
endif()

get_filename_component(BERESTA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(BERESTA_SANITIZE -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)

# ---- Инструментированная библиотека ---------------------------------------------
# ---- The instrumented library ---------------------------------------------
file(GLOB_RECURSE BERESTA_SOURCES CONFIGURE_DEPENDS "${BERESTA_ROOT}/src/*.cpp")
add_library(beresta_fuzz_lib STATIC ${BERESTA_SOURCES})
target_include_directories(beresta_fuzz_lib PUBLIC "${BERESTA_ROOT}/include")
target_compile_options(beresta_fuzz_lib PUBLIC ${BERESTA_SANITIZE} -fsanitize=fuzzer-no-link
    -Wall -Wextra -Wpedantic -Wshadow)
target_link_options(beresta_fuzz_lib PUBLIC ${BERESTA_SANITIZE})

# ---- Общие помощники целей -----------------------------------------------------
# ---- Common target helpers -----------------------------------------------------
file(GLOB_RECURSE FUZZ_COMMON CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp")
add_library(beresta_fuzz_common STATIC ${FUZZ_COMMON})
target_include_directories(beresta_fuzz_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(beresta_fuzz_common PUBLIC beresta_fuzz_lib)

# ---- Цели: каждая — своя точка входа LLVMFuzzerTestOneInput ---------------------------
# ---- Targets: each has its own LLVMFuzzerTestOneInput entry point ---------------------------
foreach(target fuzz_parser fuzz_xpath fuzz_schema)
    add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/${target}.cpp")
    target_link_libraries(${target} PRIVATE beresta_fuzz_common)
    target_link_options(${target} PRIVATE -fsanitize=fuzzer)
endforeach()
