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
# BerestaJavaSelfTest.cmake — self-test of the Java wrapper without Maven
# Called from ctest:  cmake -DJAVAC=... -DJAVA=... -DPROJECT_DIR=...
#                           -DOUTPUT_DIR=... -P BerestaJavaSelfTest.cmake
# 1) javac compiles the wrapper and self-test sources into OUTPUT_DIR/classes;
# 2) the resources (the natives folder with the binary) are copied there too — so no
#    classpath of several paths with different separators (; and :) is needed;
# 3) java runs org.beresta.selftest.SelfTest with native access allowed
#    (--enable-native-access removes the FFM API warning).
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
