#!/usr/bin/env bash
# ============================================================================
#  Сборка SDK для Linux x86_64 и aarch64 при выпуске на Gitverse
#
#  Использование (из корня исходного текста): build_linux.sh <тег vX.Y.Z> <каталог>
#  Основной путь — образ Astra Linux SE 1.7 (шаг docker:// в release.yml Gitverse):
#  GCC 8.3 и glibc 2.28, поэтому библиотека работает на Astra 1.7, РЕД ОС 7.3,
#  ALT p10 и всех более новых системах. aarch64 собирается кросс-компилятором
#  того же GCC, модульные тесты aarch64 идут в эмуляторе qemu-aarch64.
#  Запасной путь — тот же скрипт на самом раннере Ubuntu (другие имена пакетов).
# ============================================================================
#  Building the SDK for Linux x86_64 and aarch64 for a release on Gitverse
#
#  Usage (from the source root): build_linux.sh <tag vX.Y.Z> <directory>
#  The main path is the Astra Linux SE 1.7 image (the docker:// step in the Gitverse release.yml):
#  GCC 8.3 and glibc 2.28, so the library works on Astra 1.7, RED OS 7.3,
#  ALT p10 and all newer systems. aarch64 is built by the cross compiler of
#  the same GCC, the aarch64 unit tests run in the qemu-aarch64 emulator.
#  The fallback path is the same script on the Ubuntu runner itself (other package names).
# ============================================================================
set -euo pipefail

. "$(dirname "$0")/lib/build_common.sh"

# Ставит компиляторы: в Astra 1.7 кросс-пакеты с номером версии GCC, в Ubuntu — без
# него. ARM_SUFFIX — окончание имён кросс-компилятора (-8 или пусто)
# Installs the compilers: in Astra 1.7 the cross packages carry the GCC version number,
# in Ubuntu they do not. ARM_SUFFIX is the suffix of the cross compiler names (-8 or empty)
linux_tools() {
    if apt_install g++ make zip qemu-user g++-8-aarch64-linux-gnu gcc-8-aarch64-linux-gnu; then
        ARM_SUFFIX="-8"
    else
        apt_install g++ make zip qemu-user g++-aarch64-linux-gnu gcc-aarch64-linux-gnu
        ARM_SUFFIX=""
    fi
    g++ --version | head -n 1
    "aarch64-linux-gnu-g++${ARM_SUFFIX}" --version | head -n 1
}

# Сборка и тесты x86_64; в журнал — наибольшая версия glibc, которую требует .so
# The x86_64 build and tests; the log gets the highest glibc version the .so requires
build_x86_64() {
    local version="$1" out="$2"
    make -j"$(build_jobs)" all bin/libberesta.a
    make test
    echo "glibc: $(objdump -T bin/libberesta.so | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -n 1)"
    sdk_zip "$version" linux-x86_64 "$out" bin/libberesta.a bin/libberesta.so
    make fclean
}

# Кросс-сборка aarch64 и тесты в qemu (QEMU_LD_PREFIX — корень библиотек arm64)
# The aarch64 cross build and tests in qemu (QEMU_LD_PREFIX is the arm64 library root)
build_aarch64() {
    local version="$1" out="$2"
    make -j"$(build_jobs)" CXX="aarch64-linux-gnu-g++${ARM_SUFFIX}" CC="aarch64-linux-gnu-gcc${ARM_SUFFIX}" \
        AR=aarch64-linux-gnu-ar all bin/libberesta.a bin/beresta_tests
    QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 bin/beresta_tests
    sdk_zip "$version" linux-aarch64 "$out" bin/libberesta.a bin/libberesta.so
    make fclean
}

# Точка входа: тег vX.Y.Z превращается в версию X.Y.Z
# The entry point: the vX.Y.Z tag turns into the X.Y.Z version
main() {
    local version="${1#v}" out="$2"
    linux_tools
    build_x86_64 "$version" "$out"
    build_aarch64 "$version" "$out"
}

main "$@"
