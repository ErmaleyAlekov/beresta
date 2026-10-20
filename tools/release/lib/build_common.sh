#!/usr/bin/env bash
# ============================================================================
#  Общие функции сборки выпуска на Gitverse (подключается через «.»)
#
#  apt_install — установка пакетов (под root или через sudo);
#  build_jobs  — число параллельных задач сборки;
#  sdk_zip     — архив SDK: заголовки, документация, лицензия и файлы библиотеки.
#  Имя архива — beresta-<версия>-<платформа>.zip, как у релиза GitHub.
# ============================================================================
#  Common functions of the release build on Gitverse (sourced with ".")
#
#  apt_install — installing packages (as root or through sudo);
#  build_jobs  — the number of parallel build jobs;
#  sdk_zip     — the SDK archive: headers, documentation, license and library files.
#  The archive name is beresta-<version>-<platform>.zip, as in the GitHub release.
# ============================================================================

# Ставит пакеты apt; код возврата — как у apt-get install (пакета нет — не 0)
# Installs apt packages; the exit code is that of apt-get install (no package — not 0)
apt_install() {
    local sudo=""
    if [ "$(id -u)" != "0" ]; then
        sudo="sudo"
    fi
    $sudo apt-get update -qq
    $sudo env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq "$@"
}

# Печатает число процессоров (по нему make -j и cmake --build -j)
# Prints the number of processors (used by make -j and cmake --build -j)
build_jobs() {
    nproc 2> /dev/null || echo 2
}

# Собирает beresta-<версия>-<платформа>.zip в каталоге вывода
# sdk_zip <версия> <платформа> <каталог вывода> <файлы библиотеки...>
# Builds beresta-<version>-<platform>.zip in the output directory
# sdk_zip <version> <platform> <output directory> <library files...>
sdk_zip() {
    local version="$1" platform="$2" out="$3" name stage out_abs
    shift 3
    name="beresta-${version}-${platform}"
    stage="$(mktemp -d)"
    mkdir -p "${stage}/${name}/lib" "$out"
    cp -R include LICENSE README.md README.en.md CHANGELOG.md CHANGELOG.en.md docs "${stage}/${name}/"
    cp "$@" "${stage}/${name}/lib/"
    out_abs="$(cd "$out" && pwd)"
    # Архив собирается из временного каталога, чтобы внутри был один корень <name>/
    # The archive is built from the temporary directory so that it has one root <name>/
    (cd "$stage" && zip -qr "${out_abs}/${name}.zip" "$name")
    rm -rf "$stage"
    echo "SDK: ${out}/${name}.zip"
}
