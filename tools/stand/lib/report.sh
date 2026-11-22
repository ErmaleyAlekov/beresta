# ============================================================================
#  Файл для создания отчёта и запуска шагов проверочного стенда «Берёсты»
#  Функции для начала отчёта, записи строк, выполнения шагов и завершения отчёта.
# ============================================================================
#  File for creating a report and running steps of the «Beresta» test stand
#  Functions for starting the report, writing lines, running steps, and finishing the report.
# ============================================================================

# Начинает отчёт, создаёт директорию для логов и перезаписывает файл отчёта
# Starts the report, creates a directory for logs and overwrites the report file
report_begin() {
    mkdir -p "$STAND_LOGS"
    printf '%s\n' "$(stand_text title)" > "$STAND_REPORT"
    printf '%s: %s\n\n' "$(stand_text started)" "$(date '+%Y-%m-%d %H:%M:%S %z')" >> "$STAND_REPORT"
    printf '%s\n' "$(stand_text title)"
    printf '%s: %s\n\n' "$(stand_text started)" "$(date '+%Y-%m-%d %H:%M:%S %z')"
}

# Записывает строку в отчёт и выводит её в stdout
# Writes a line to the report and prints it to stdout
report_line() {
    printf '%s: %s\n' "$(stand_text "$1")" "$2" >> "$STAND_REPORT"
    printf '%s: %s\n' "$(stand_text "$1")" "$2"
}

# Выполняет шаг, записывает его время выполнения и результат в отчёт
# Runs a step, records its execution time and result in the report
run_step() {
    # Префикс переменных отчёта отделяет их от переменных функций шагов в POSIX sh.
    # The report variable prefix keeps these variables distinct from step variables in POSIX sh.
    report_step_start=$(date +%s)
    report_step_status=OK
    if ! "$2" > "$STAND_LOGS/$1.log" 2>&1; then
        report_step_status=FAIL
    fi
    report_step_end=$(date +%s)
    report_step_duration=$((report_step_end - report_step_start))
    printf '[%s] %s (%d %s)\n' "$report_step_status" "$(stand_text "$3")" "$report_step_duration" "$(stand_text seconds)" >> "$STAND_REPORT"
    printf '[%s] %s (%d %s)\n' "$report_step_status" "$(stand_text "$3")" "$report_step_duration" "$(stand_text seconds)"
    if [ "$report_step_status" = FAIL ]; then
        printf '%s:\n' "$(stand_text log_tail)" >> "$STAND_REPORT"
        printf '%s:\n' "$(stand_text log_tail)"
        tail -n 20 "$STAND_LOGS/$1.log" >> "$STAND_REPORT"
        tail -n 20 "$STAND_LOGS/$1.log"
        STAND_FAILED=$((STAND_FAILED + 1))
        return 1
    fi
}

# Завершает отчёт, выводит итоги и сохраняет файл отчёта
# Finishes the report, prints the summary and saves the report file
report_finish() {
    printf '\n' >> "$STAND_REPORT"
    printf '%s: %s\n' "$(stand_text finished)" "$(date '+%Y-%m-%d %H:%M:%S %z')" >> "$STAND_REPORT"
    printf '%s: %s\n' "$(stand_text finished)" "$(date '+%Y-%m-%d %H:%M:%S %z')"

    if [ "$STAND_FAILED" -eq 0 ]; then
        printf '%s\n' "$(stand_text summary_ok)" >> "$STAND_REPORT"
        printf '%s\n' "$(stand_text summary_ok)"
    else
        printf '%s: %d\n' "$(stand_text summary_fail)" "$STAND_FAILED" >> "$STAND_REPORT"
        printf '%s: %d\n' "$(stand_text summary_fail)" "$STAND_FAILED"
    fi

    printf '%s: %s\n' "$(stand_text report_saved)" "$STAND_REPORT" >> "$STAND_REPORT"
    printf '%s: %s\n' "$(stand_text report_saved)" "$STAND_REPORT"

    return "$STAND_FAILED"
}

# Переносит в отчёт итог модульных тестов из журнала шага (строка «Итого:» у
# make test, «tests passed» у ctest): протокол показывает число пройденных тестов
# Copies the unit test summary from the step log into the report (the «Итого:» line
# of make test, «tests passed» of ctest): the protocol shows the number of passed tests
report_test_total() {
    total="$(grep -E 'Итого:|tests passed' "$STAND_LOGS/test.log" 2>/dev/null | tail -n 1)"
    if [ -n "$total" ]; then
        report_line test_total "${total#Итого: }"
    fi
}
