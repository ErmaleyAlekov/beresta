# ============================================================================
#  BerestaWasm.cmake — сборка WebAssembly (Emscripten)
#
#    emcmake cmake -S . -B build-wasm -DBERESTA_BUILD_TESTS=ON
#    cmake --build build-wasm
#    ctest --test-dir build-wasm --output-on-failure
#
#  Объекты ядра компонуются в модуль ES bindings/wasm/dist/beresta_core.mjs +
#  beresta_core.wasm, затем tools/bundle.mjs собирает поверх него общий
#  JS-слой в dist/beresta.mjs. Флаги совпадают с целью make wasm.
#  Динамическая библиотека, обёртки и программы для Wasm не собираются:
#  у браузера нет ни dlopen, ни файловой системы.
# ============================================================================
#  BerestaWasm.cmake — the WebAssembly build (Emscripten)
#
#    emcmake cmake -S . -B build-wasm -DBERESTA_BUILD_TESTS=ON
#    cmake --build build-wasm
#    ctest --test-dir build-wasm --output-on-failure
#
#  The core objects are linked into the ES module bindings/wasm/dist/beresta_core.mjs +
#  beresta_core.wasm, then tools/bundle.mjs builds the common JS layer on top of it
#  into dist/beresta.mjs. The flags match the make wasm target.
#  The shared library, the wrappers and the programs are not built for Wasm:
#  a browser has neither dlopen nor a file system.
# ============================================================================

# Флаги компоновки модуля (смысл каждого — в Makefile, раздел WebAssembly)
# The module link flags (the meaning of each one is in the Makefile, the WebAssembly section)
set(BERESTA_WASM_LINK_FLAGS
    -fwasm-exceptions
    "SHELL:-sMODULARIZE=1" "SHELL:-sEXPORT_ES6=1" "SHELL:-sEXPORT_NAME=createBerestaCore"
    "SHELL:-sENVIRONMENT=web,worker,node" "SHELL:-sALLOW_MEMORY_GROWTH=1" "SHELL:-sWASM_BIGINT=1"
    "SHELL:-sFILESYSTEM=0" "SHELL:-sSTACK_SIZE=1048576"
    "SHELL:-sEXPORTED_FUNCTIONS=@${PROJECT_SOURCE_DIR}/bindings/wasm/exports.json"
    "SHELL:-sEXPORTED_RUNTIME_METHODS=UTF8ToString,stringToUTF8,lengthBytesUTF8,getValue,HEAPU8")

function(beresta_setup_wasm)
    set(package_dir "${PROJECT_SOURCE_DIR}/bindings/wasm")
    target_compile_options(beresta_objects PRIVATE -fwasm-exceptions)
    add_executable(beresta_wasm $<TARGET_OBJECTS:beresta_objects>)
    set_target_properties(beresta_wasm PROPERTIES OUTPUT_NAME beresta_core SUFFIX ".mjs"
        RUNTIME_OUTPUT_DIRECTORY "${package_dir}/dist"
        LINK_DEPENDS "${package_dir}/exports.json")
    target_link_options(beresta_wasm PRIVATE ${BERESTA_WASM_LINK_FLAGS})
    find_program(BERESTA_NODE_EXECUTABLE NAMES node)
    if(NOT BERESTA_NODE_EXECUTABLE)
        message(WARNING "Берёста: node не найден — dist/beresta.mjs не будет собран / "
            "node was not found — dist/beresta.mjs will not be built")
        return()
    endif()
    add_custom_command(TARGET beresta_wasm POST_BUILD
        COMMAND "${BERESTA_NODE_EXECUTABLE}" "${package_dir}/tools/bundle.mjs"
        COMMENT "Берёста: сборка dist/beresta.mjs / building dist/beresta.mjs" VERBATIM)
    if(BERESTA_BUILD_TESTS)
        add_test(NAME beresta_wasm_selftest COMMAND "${BERESTA_NODE_EXECUTABLE}" --test
            WORKING_DIRECTORY "${package_dir}")
    endif()
endfunction()
