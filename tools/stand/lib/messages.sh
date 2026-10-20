# ============================================================================
#  Каталог сообщений для проверочного стенда «Берёсты»
#  Функция stand_text ID печатает текст сообщения ID на языке STAND_LANG.
# ============================================================================
#  Message catalog for the «Beresta» test stand
#  The stand_text ID function prints the text of message ID in the STAND_LANG language.
# ============================================================================

stand_text() {
    case "$1" in
        title)
            ru='Проверочный стенд «Берёсты»'
            en='Beresta test stand'
            ;;
        env_os)
            ru='ОС'
            en='OS'
            ;;
        env_kernel)
            ru='Ядро'
            en='Kernel'
            ;;
        env_glibc)
            ru='glibc'
            en='glibc'
            ;;
        env_compiler)
            ru='Компилятор C++'
            en='C++ Compiler'
            ;;
        env_make)
            ru='make'
            en='make'
            ;;
        env_cmake)
            ru='cmake'
            en='cmake'
            ;;
        env_note)
            ru='Примечание'
            en='Note'
            ;;
        not_found)
            ru='не найден'
            en='not found'
            ;;
        step_cxx17)
            ru='Проверка поддержки C++17'
            en='Checking C++17 support'
            ;;
        step_static)
            ru='Проверка статической libstdc++'
            en='Checking static libstdc++'
            ;;
        step_build)
            ru='Сборка библиотеки'
            en='Building the library'
            ;;
        step_test)
            ru='Модульные тесты'
            en='Unit tests'
            ;;
        test_total)
            ru='Итог тестов'
            en='Test summary'
            ;;
        step_demo)
            ru='Демонстрационные программы'
            en='Demonstration programs'
            ;;
        ok)
            ru='успешно'
            en='successfully'
            ;;
        fail)
            ru='ошибка'
            en='error'
            ;;
        seconds)
            ru='с'
            en='s'
            ;;
        no_compiler)
            ru='Не найден компилятор C++: установите g++ 9+ или clang++ 9+'
            en='C++ compiler not found: install g++ 9+ or clang++ 9+'
            ;;
        no_builder)
            ru='Не найдены ни make, ни cmake: установите make'
            en='Neither make nor cmake found: install make'
            ;;
        gcc_old)
            ru='GCC до 9-й версии: std::filesystem подключается из libstdc++fs (-lstdc++fs)'
            en='GCC older than 9: std::filesystem is linked from libstdc++fs (-lstdc++fs)'
            ;;
        static_hint)
            ru='Нет статической libstdc++: РЕД ОС — dnf install libstdc++-static, ALT — пакет libstdc++<версия GCC>-devel-static'
            en='No static libstdc++: RED OS — dnf install libstdc++-static, ALT — the libstdc++<GCC version>-devel-static package'
            ;;
        log_tail)
            ru='Последние строки журнала'
            en='Last lines of the log'
            ;;
        summary_ok)
            ru='Все шаги пройдены'
            en='All steps completed'
            ;;
        summary_fail)
            ru='Проваленных шагов'
            en='Failed steps'
            ;;
        report_saved)
            ru='Отчёт сохранён'
            en='Report saved'
            ;;
        started)
            ru='Начало'
            en='Start'
            ;;
        finished)
            ru='Окончание'
            en='Finish'
            ;;
        usage)
            ru='Использование: sh tools/stand/run_stand.sh. Переменные: CXX, CC — компиляторы; STAND_LANG=ru|en — язык'
            en='Usage: sh tools/stand/run_stand.sh. Variables: CXX, CC — compilers; STAND_LANG=ru|en — language'
            ;;
        *)
            ru="Неизвестное сообщение: $1"
            en="Unknown message: $1"
            ;;
    esac

    if [ "$STAND_LANG" = "en" ]; then
        printf '%s\n' "$en"
    else
        printf '%s\n' "$ru"
    fi
}
