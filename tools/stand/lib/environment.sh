# ============================================================================
#  Определение переменных окружения для проверочного стенда «Берёсты»
#  Определяет компиляторы, make и cmake, проверяет их наличие и версии.
# ============================================================================
#  Environment setup for the «Beresta» test stand
#  Defines compilers, make, and cmake, checks their presence and versions.
# ============================================================================

# Первая из перечисленных программ, найденная в PATH (имя, а не полный путь:
# путь с пробелами, как «C:/Program Files», ломает команды make)
# The first of the listed programs found in PATH (the name, not the full path:
# a path with spaces, like "C:/Program Files", breaks the make commands)
env_first_tool() {
    for tool in "$@"; do
        if command -v "$tool" >/dev/null 2>&1; then
            printf '%s
' "$tool"
            return 0
        fi
    done
    return 1
}

# Находит компиляторы, make и cmake (CXX и CC из окружения важнее найденных)
# Finds the compilers, make and cmake (CXX and CC from the environment win over the found ones)
env_find_tools() {
    STAND_CXX="${CXX:-$(env_first_tool c++ g++ clang++)}"
    STAND_CC="${CC:-$(env_first_tool cc gcc clang)}"
    STAND_MAKE="$(env_first_tool make gmake)"
    STAND_CMAKE="$(env_first_tool cmake)"

    if [ -z "$STAND_CXX" ]; then
        stand_text no_compiler >&2
        return 1
    fi

    if [ -z "$STAND_MAKE" ] && [ -z "$STAND_CMAKE" ]; then
        stand_text no_builder >&2
        return 1
    fi
}

# Описывает текущее окружение: ОС, ядро, glibc, компиляторы, make, cmake
# Describes the current environment: OS, kernel, glibc, compilers, make, cmake
env_describe() {
    os_name="$(env_os_name)"
    kernel_info="$(uname -srm)"
    glibc_version="$(getconf GNU_LIBC_VERSION 2>/dev/null)" || glibc_version="$(ldd --version 2>/dev/null | head -n 1)" || glibc_version="$(stand_text not_found)"
    compiler_version="$("$STAND_CXX" --version 2>/dev/null | head -n 1)"
    make_version="$(stand_text not_found)"
    if [ -n "$STAND_MAKE" ]; then
        make_version="$("$STAND_MAKE" --version 2>/dev/null | head -n 1)"
    fi
    cmake_version="$(stand_text not_found)"
    if [ -n "$STAND_CMAKE" ]; then
        cmake_version="$("$STAND_CMAKE" --version 2>/dev/null | head -n 1)"
    fi

    report_line env_os "$os_name"
    report_line env_kernel "$kernel_info"
    report_line env_glibc "$glibc_version"
    report_line env_compiler "$compiler_version"
    report_line env_make "$make_version"
    report_line env_cmake "$cmake_version"

    # Мажорная версия GCC — из -dumpversion (у clang в --version есть слово clang)
    # The GCC major version comes from -dumpversion (clang has the word clang in --version)
    major="$("$STAND_CXX" -dumpversion 2>/dev/null | cut -d'.' -f1)"
    if ! echo "$compiler_version" | grep -qi 'clang' && [ -n "$major" ] && [ "$major" -lt 9 ] 2>/dev/null; then
        report_line env_note "$(stand_text gcc_old)"
    fi
}

# Проверяет поддержку C++17 компилятором
# Checks C++17 support by the compiler
env_check_cxx17() {
    temp_dir=$(mktemp -d)
    cat > "$temp_dir/probe.cpp" <<EOF
#include <optional>
#include <string_view>
#include <filesystem>
#include <iostream>
#include <tuple>

int main() {
    std::optional<int> opt = 42;
    std::string_view sv = "hello";
    std::filesystem::path p = "/tmp";
    if constexpr (true) {
        std::cout << "C++17 features are supported." << std::endl;
    }
    auto [a, b] = std::make_tuple(1, 2);
    return 0;
}
EOF

    # На GCC 8 std::filesystem лежит в libstdc++fs: вторая попытка — с ней
    # (так же поступают Makefile и CMakeLists.txt)
    # On GCC 8 std::filesystem lives in libstdc++fs: the second attempt uses it
    # (the Makefile and CMakeLists.txt do the same)
    if "$STAND_CXX" -std=c++17 "$temp_dir/probe.cpp" -o "$temp_dir/probe" 2>/dev/null \
        || "$STAND_CXX" -std=c++17 "$temp_dir/probe.cpp" -o "$temp_dir/probe" -lstdc++fs 2>/dev/null; then
        "$temp_dir/probe" 2>/dev/null
        result=$?
    else
        result=1
    fi

    rm -rf "$temp_dir"
    return $result
}

# Проверяет наличие статической библиотеки libstdc++
# Checks for the presence of static libstdc++
env_check_static_libstdcxx() {
    if echo "$(uname -s)" | grep -q 'MINGW\|MSYS\|CYGWIN'; then
        return 0
    fi

    temp_dir=$(mktemp -d)
    cat > "$temp_dir/lib.cpp" <<EOF
#include <string>

extern "C" {
    void lib_func() {
        std::string str = "test";
    }
}
EOF

    "$STAND_CXX" -std=c++17 -shared -fPIC -static-libstdc++ -static-libgcc "$temp_dir/lib.cpp" -o "$temp_dir/lib.so" 2>/dev/null
    if [ $? -ne 0 ]; then
        stand_text static_hint >&2
        rm -rf "$temp_dir"
        return 1
    fi

    rm -rf "$temp_dir"
    return 0
}
