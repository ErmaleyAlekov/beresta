# ============================================================================
#  BerestaBindings.cmake — обёртки для Java, C# и Python
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
