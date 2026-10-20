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
    local name="$1"
    local func="$2"
    local title="$3"
    local start_time
    local end_time
    local duration

    start_time=$(date +%s)
    "$func" > "$STAND_LOGS/$name.log" 2>&1
    # Код шага запоминается сразу: следующие команды его перезапишут
    # The step code is saved at once: the following commands would overwrite it
    status=$?
    end_time=$(date +%s)
    duration=$((end_time - start_time))

    if [ "$status" -eq 0 ]; then
        printf '[OK] %s (%d %s)\n' "$(stand_text "$title")" "$duration" "$(stand_text seconds)" >> "$STAND_REPORT"
        printf '[OK] %s (%d %s)\n' "$(stand_text "$title")" "$duration" "$(stand_text seconds)"
    else
        printf '[FAIL] %s (%d %s)\n' "$(stand_text "$title")" "$duration" "$(stand_text seconds)" >> "$STAND_REPORT"
        printf '[FAIL] %s (%d %s)\n' "$(stand_text "$title")" "$duration" "$(stand_text seconds)"
        printf '%s:\n' "$(stand_text log_tail)" >> "$STAND_REPORT"
        printf '%s:\n' "$(stand_text log_tail)"
        tail -n 20 "$STAND_LOGS/$name.log" >> "$STAND_REPORT"
        tail -n 20 "$STAND_LOGS/$name.log"
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
