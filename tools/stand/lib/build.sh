# ============================================================================
#  Файл: lib/build.sh
#  Описание: Функции для сборки и проверки библиотеки «Берёсты».
# ============================================================================
#  File: lib/build.sh
#  Description: Functions for building and testing the «Beresta» library.
# ============================================================================

# Печатает число процессоров для параллельной сборки
# Prints the number of processors for parallel build
build_jobs() {
    if command -v nproc >/dev/null; then
        nproc
    elif command -v getconf >/dev/null; then
        getconf _NPROCESSORS_ONLN
    else
        printf '2\n'
    fi
}

# Собирает библиотеку
# Builds the library
build_library() {
    (
        cd "$STAND_ROOT" || exit 1
        if [ -n "$STAND_MAKE" ]; then
            "$STAND_MAKE" -j"$(build_jobs)" CXX="$STAND_CXX" CC="$STAND_CC" all
        else
            "$STAND_CMAKE" -S . -B build-stand -DCMAKE_BUILD_TYPE=Release -DBERESTA_BUILD_TESTS=ON -DBERESTA_BUILD_EXAMPLES=ON
            "$STAND_CMAKE" --build build-stand -j "$(build_jobs)"
        fi
    )
}

# Запускает модульные тесты
# Runs unit tests
build_run_tests() {
    (
        cd "$STAND_ROOT" || exit 1
        if [ -n "$STAND_MAKE" ]; then
            "$STAND_MAKE" CXX="$STAND_CXX" CC="$STAND_CC" test
        else
            ctest --test-dir build-stand --output-on-failure
        fi
    )
}

# Путь к демо-программе $1 (с .exe на Windows); пусто, если её нет
# The path to the $1 demo program (with .exe on Windows); empty if there is none
build_find_demo() {
    if [ -n "$STAND_MAKE" ]; then
        for candidate in "$STAND_ROOT/bin/$1" "$STAND_ROOT/bin/$1.exe"; do
            if [ -f "$candidate" ]; then
                printf '%s\n' "$candidate"
                return 0
            fi
        done
        return 1
    fi
    find "$STAND_ROOT/build-stand" -type f \( -name "$1" -o -name "$1.exe" \) | head -n 1
}

# Запускает демо C++ и C из их каталогов; нет программы или она упала — код 1
# Runs the C++ and C demos from their directories; a missing or failed program gives code 1
build_run_demos() {
    for name in beresta_demo beresta_capi_demo; do
        demo="$(build_find_demo "$name")"
        if [ -z "$demo" ]; then
            printf '%s: %s\n' "$name" "$(stand_text not_found)"
            return 1
        fi
        printf '=== %s\n' "$demo"
        if ! (cd "$(dirname "$demo")" && "./$(basename "$demo")"); then
            return 1
        fi
    done
}
