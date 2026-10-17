# ============================================================================
#  BerestaJavaSelfTest.cmake — самопроверка Java-обёртки без Maven
#
#  Вызывается из ctest:  cmake -DJAVAC=... -DJAVA=... -DPROJECT_DIR=...
#                              -DOUTPUT_DIR=... -P BerestaJavaSelfTest.cmake
#
#  1) javac компилирует исходники обёртки и самопроверки в OUTPUT_DIR/classes;
#  2) ресурсы (папка natives с бинарником) копируются туда же — так не нужен
#     classpath из нескольких путей с разными разделителями (; и :);
#  3) java запускает org.beresta.selftest.SelfTest с разрешённым нативным
#     доступом (--enable-native-access убирает предупреждение FFM API).
# ============================================================================

file(GLOB_RECURSE sources
    "${PROJECT_DIR}/src/main/java/*.java"
    "${PROJECT_DIR}/src/test/java/*.java")
set(classes "${OUTPUT_DIR}/classes")
file(REMOVE_RECURSE "${classes}")
file(MAKE_DIRECTORY "${classes}")

execute_process(COMMAND "${JAVAC}" --release 22 -encoding UTF-8 -d "${classes}" ${sources}
    RESULT_VARIABLE compile_result)
if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR "Берёста: javac завершился с кодом ${compile_result}")
endif()

file(COPY "${PROJECT_DIR}/src/main/resources/" DESTINATION "${classes}")

execute_process(COMMAND "${JAVA}" --enable-native-access=ALL-UNNAMED -Dstdout.encoding=UTF-8 -cp "${classes}"
    org.beresta.selftest.SelfTest
    RESULT_VARIABLE run_result)
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Берёста: самопроверка Java завершилась с кодом ${run_result}")
endif()
