# ============================================================================
#  BerestaBindings.cmake — обёртки для Java, C#, Python и Node.js
#
#  Все три обёртки работают с ОДНИМ И ТЕМ ЖЕ бинарником — динамической
#  библиотекой beresta с плоским Си-интерфейсом. Задача CMake — после каждой
#  сборки положить этот бинарник туда, где его найдёт загрузчик обёртки:
#
#    Java   bindings/java/src/main/resources/natives/<os>-<arch>/
#           (Maven упакует его в jar; загрузчик извлечёт и подключит
#           через Project Panama — SymbolLookup.libraryLookup)
#    C#     bindings/csharp/Beresta/runtimes/<rid>/native/
#           (стандартная раскладка NuGet: .NET сам выберет нужный RID)
#    Python bindings/python/beresta/_native/<os>-<arch>/
#           (ctypes.CDLL загрузит файл по абсолютному пути)
#
#  Паттерн: Template Method на уровне сборки — общий шаг «скопировать
#  бинарник» (beresta_copy_native_library) и частные шаги каждой обёртки
#  (упаковка Maven, dotnet build, тесты).
# ============================================================================

set(BERESTA_BINDINGS_DIR "${PROJECT_SOURCE_DIR}/bindings")

# ---- Общий шаг: копирование собранной библиотеки в папку обёртки -------------
# POST_BUILD-команды накапливаются: при нескольких флагах бинарник
# копируется во все выбранные места. copy_if_different не трогает файл,
# если он не изменился, — Maven и dotnet не пересобирают лишнего.
# ---- The common step: copying the built library into the wrapper folder ------
# POST_BUILD commands accumulate: with several flags the binary is
# copied to all selected places. copy_if_different does not touch the file
# if it has not changed — Maven and dotnet do not rebuild anything extra.
function(beresta_copy_native_library target destination)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${destination}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${target}>" "${destination}"
        COMMENT "Берёста: копирование $<TARGET_FILE_NAME:${target}> в ${destination}"
        VERBATIM)
endfunction()

# ---- Java (Project Panama, Java 22+) ------------------------------------------
# Самопроверка запускается без Maven: javac + java из JDK (скрипт
# BerestaJavaSelfTest.cmake), поэтому ctest работает и без доступа к
# репозиторию Maven Central. Упаковка в jar — отдельная цель beresta_java_package.
# ---- Java (Project Panama, Java 22+) ------------------------------------------
# The self-test runs without Maven: javac + java from the JDK (the
# BerestaJavaSelfTest.cmake script), so ctest works even without access to
# the Maven Central repository. Packaging into a jar is a separate target beresta_java_package.
function(beresta_setup_java)
    set(BERESTA_JAVA_PROJECT_DIR "${BERESTA_BINDINGS_DIR}/java" CACHE PATH "Maven-проект Java-обёртки")
    set(resources "${BERESTA_JAVA_PROJECT_DIR}/src/main/resources/natives/${BERESTA_PLATFORM}")
    beresta_copy_native_library(beresta "${resources}")
    find_program(BERESTA_MAVEN_EXECUTABLE NAMES mvn mvn.cmd)
    if(BERESTA_MAVEN_EXECUTABLE)
        add_custom_target(beresta_java_package
            COMMAND "${BERESTA_MAVEN_EXECUTABLE}" -B -q -f "${BERESTA_JAVA_PROJECT_DIR}/pom.xml" package
            DEPENDS beresta
            COMMENT "Берёста: сборка jar с нативной библиотекой для ${BERESTA_PLATFORM}"
            VERBATIM)
    endif()
    find_package(Java 22 COMPONENTS Development Runtime)
    if(BERESTA_BUILD_TESTS AND Java_FOUND)
        add_test(NAME beresta_java_selftest COMMAND "${CMAKE_COMMAND}"
            "-DJAVAC=${Java_JAVAC_EXECUTABLE}" "-DJAVA=${Java_JAVA_EXECUTABLE}"
            "-DPROJECT_DIR=${BERESTA_JAVA_PROJECT_DIR}" "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/java-selftest"
            -P "${PROJECT_SOURCE_DIR}/cmake/BerestaJavaSelfTest.cmake")
    endif()
endfunction()

# ---- C# (.NET, P/Invoke) -------------------------------------------------------
function(beresta_setup_csharp)
    set(project_dir "${BERESTA_BINDINGS_DIR}/csharp")
    beresta_copy_native_library(beresta "${project_dir}/Beresta/runtimes/${BERESTA_DOTNET_RID}/native")
    find_program(BERESTA_DOTNET_EXECUTABLE NAMES dotnet)
    if(NOT BERESTA_DOTNET_EXECUTABLE)
        message(STATUS "Берёста: dotnet не найден — C#-обёртка только получит бинарник")
        return()
    endif()
    add_custom_target(beresta_csharp_package
        COMMAND "${BERESTA_DOTNET_EXECUTABLE}" pack "${project_dir}/Beresta/Beresta.csproj" -c Release
        DEPENDS beresta
        COMMENT "Берёста: сборка NuGet-пакета с нативной библиотекой для ${BERESTA_DOTNET_RID}"
        VERBATIM)
    if(BERESTA_BUILD_TESTS)
        add_test(NAME beresta_csharp_selftest COMMAND "${BERESTA_DOTNET_EXECUTABLE}" run
            --project "${project_dir}/Beresta.SelfTest/Beresta.SelfTest.csproj" -c Release)
    endif()
endfunction()

# ---- Python (ctypes) ------------------------------------------------------------
function(beresta_setup_python)
    set(package_dir "${BERESTA_BINDINGS_DIR}/python")
    beresta_copy_native_library(beresta "${package_dir}/beresta/_native/${BERESTA_PLATFORM}")
    find_package(Python3 COMPONENTS Interpreter)
    if(BERESTA_BUILD_TESTS AND Python3_FOUND)
        add_test(NAME beresta_python_selftest
            COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tests
            WORKING_DIRECTORY "${package_dir}")
    endif()
endfunction()

# ---- Node.js (Node-API) ---------------------------------------------------------
# Аддон beresta.node — отдельный модуль: переходники Node-API плюс те же объекты
# ядра, что и в динамической библиотеке (один самодостаточный файл в пакете
# npm). Заголовки Node и node-gyp не нужны: функции Node-API аддон находит в
# процессе Node сам. Файл кладётся в свой каталог сборки, чтобы на MSVC его
# .lib / .exp не столкнулись с библиотекой импорта beresta.lib.
# ---- Node.js (Node-API) ---------------------------------------------------------
# The beresta.node addon is a separate module: the Node-API adapters plus the same
# core objects as in the shared library (one self-contained file in the npm
# package). Neither the Node headers nor node-gyp are needed: the addon finds the
# Node-API functions in the Node process itself. The file goes into its own build
# directory so that on MSVC its .lib / .exp do not clash with the beresta.lib import library.
function(beresta_setup_node)
    set(package_dir "${BERESTA_BINDINGS_DIR}/node")
    file(GLOB_RECURSE addon_sources CONFIGURE_DEPENDS "${package_dir}/addon/src/*.cpp")
    add_library(beresta_node MODULE ${addon_sources} $<TARGET_OBJECTS:beresta_objects>)
    target_include_directories(beresta_node PRIVATE "${PROJECT_SOURCE_DIR}/include" "${package_dir}/addon/include")
    target_link_libraries(beresta_node PRIVATE ${CMAKE_DL_LIBS})
    set_target_properties(beresta_node PROPERTIES
        OUTPUT_NAME beresta PREFIX "" SUFFIX ".node"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/node"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/node"
        POSITION_INDEPENDENT_CODE ON CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
    beresta_apply_compile_options(beresta_node)
    beresta_apply_link_options(beresta_node)
    beresta_copy_native_library(beresta_node "${package_dir}/native/${BERESTA_PLATFORM}")
    find_program(BERESTA_NODE_EXECUTABLE NAMES node)
    find_program(BERESTA_NPM_EXECUTABLE NAMES npm npm.cmd)
    if(BERESTA_NPM_EXECUTABLE)
        add_custom_target(beresta_node_package COMMAND "${BERESTA_NPM_EXECUTABLE}" pack
            WORKING_DIRECTORY "${package_dir}" DEPENDS beresta_node
            COMMENT "Берёста: сборка пакета npm с аддоном для ${BERESTA_PLATFORM}" VERBATIM)
    endif()
    if(BERESTA_BUILD_TESTS AND BERESTA_NODE_EXECUTABLE)
        add_test(NAME beresta_node_selftest COMMAND "${BERESTA_NODE_EXECUTABLE}" --test
            WORKING_DIRECTORY "${package_dir}")
    endif()
endfunction()
