#!/bin/sh
# ============================================================================
#  Точка входа для проверочного стенда «Берёсты»
#  Запускает сборку библиотеки, модульные тесты и демонстрационные программы.
# ============================================================================
#  Entry point for the «Beresta» test stand
#  Runs the library build, unit tests, and demonstration programs.
# ============================================================================

# Вычисление путей
# Calculation of paths
STAND_DIR=$(cd "$(dirname "$0")" && pwd)
STAND_ROOT=$(dirname "$(dirname "$STAND_DIR")")
STAND_LOGS="$STAND_ROOT/stand_logs"
STAND_REPORT="$STAND_ROOT/stand_report.txt"
STAND_LANG=${STAND_LANG:-ru}
STAND_FAILED=0

# Подключение библиотек
# Include libraries
. "$STAND_DIR/lib/messages.sh"
. "$STAND_DIR/lib/report.sh"
. "$STAND_DIR/lib/environment.sh"
. "$STAND_DIR/lib/build.sh"

# Помощь
# Help
usage() {
    stand_text usage
    exit 0
}

# Обработка аргументов
# Argument processing
while [ "$#" -gt 0 ]; do
    case "$1" in
        --help)
            usage
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

# Основная функция
# Main function
main() {
    report_begin
    if ! env_find_tools; then
        report_finish
        exit 1
    fi
    env_describe
    if ! run_step cxx17 env_check_cxx17 step_cxx17; then
        report_finish
        exit 1
    fi
    run_step static env_check_static_libstdcxx step_static
    if ! run_step build build_library step_build; then
        report_finish
        exit 1
    fi
    run_step test build_run_tests step_test
    run_step demo build_run_demos step_demo
    report_finish
    exit $STAND_FAILED
}

# Запуск
# Run
main
