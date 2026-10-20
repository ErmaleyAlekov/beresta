#!/usr/bin/env bash
# ============================================================================
#  Скачивание файлов релиза GitHub для зеркалирования на Gitverse
#
#  Использование: github_assets.sh <тег> <каталог> [минут ожидания]
#  Релиз на GitHub собирает release.yml (пять платформ, ~20–30 минут). Скрипт
#  ждёт, пока релиз появится в открытом API, и скачивает все его файлы в каталог.
#  gh release create публикует релиз только после загрузки всех файлов, поэтому
#  видимый релиз уже полный. Не дождался — код 3, каталог остаётся пустым.
#  Нужны только curl и python3; токен GitHub не нужен (репозиторий открытый).
# ============================================================================
#  Downloading the GitHub release files to mirror them on Gitverse
#
#  Usage: github_assets.sh <tag> <directory> [minutes to wait]
#  The GitHub release is built by release.yml (five platforms, ~20–30 minutes). The script
#  waits until the release appears in the public API and downloads all its files into the directory.
#  gh release create publishes a release only after all files are uploaded, so a
#  visible release is already complete. Not waited out — exit code 3, the directory stays empty.
#  Only curl and python3 are needed; no GitHub token (the repository is public).
# ============================================================================
set -euo pipefail

# Репозиторий на GitHub можно переопределить переменной окружения
# The GitHub repository can be overridden by an environment variable
GH_REPOSITORY="${GH_REPOSITORY:-ErmaleyAlekov/beresta}"
# Интерпретатор Python (в Windows обычно python, а не python3)
# The Python interpreter (on Windows usually python, not python3)
PYTHON="${PYTHON:-python3}"

# Пишет JSON релиза по тегу в файл; код 0 — релиз есть, иначе — нет
# Writes the release JSON by tag into a file; code 0 — the release exists, otherwise — not
release_json() {
    local tag="$1" out="$2" code
    code=$(curl -s -o "$out" -w '%{http_code}' \
        -H 'Accept: application/vnd.github+json' \
        "https://api.github.com/repos/${GH_REPOSITORY}/releases/tags/${tag}")
    [ "$code" = "200" ]
}

# Опрашивает API раз в минуту, пока релиз не появится или не выйдет время
# Polls the API once a minute until the release appears or the time runs out
wait_release() {
    local tag="$1" out="$2" minutes="$3" attempt=0
    while ! release_json "$tag" "$out"; do
        attempt=$((attempt + 1))
        if [ "$attempt" -gt "$minutes" ]; then
            echo "Релиз ${tag} на GitHub не появился за ${minutes} мин / The GitHub release did not appear in ${minutes} min"
            return 3
        fi
        echo "Жду релиз ${tag} на GitHub (${attempt}/${minutes}) / Waiting for the GitHub release"
        sleep 60
    done
}

# Скачивает каждый файл релиза по его browser_download_url
# Downloads every release file by its browser_download_url
download_assets() {
    local json="$1" dir="$2" url
    mkdir -p "$dir"
    # Python в Windows завершает строки CR LF — tr срезает CR
    # Python on Windows ends lines with CR LF — tr cuts the CR off
    "$PYTHON" -c 'import json,sys; [print(a["browser_download_url"]) for a in json.load(open(sys.argv[1]))["assets"]]' "$json" |
    tr -d '\r' |
    while read -r url; do
        echo "↓ ${url##*/}"
        curl -sfL -o "${dir}/${url##*/}" "$url"
    done
}

# Точка входа: тег, каталог, минуты ожидания (по умолчанию 20)
# The entry point: tag, directory, minutes to wait (20 by default)
main() {
    local tag="$1" dir="$2" minutes="${3:-20}" json
    json="$(mktemp)"
    wait_release "$tag" "$json" "$minutes"
    download_assets "$json" "$dir"
    rm -f "$json"
}

main "$@"
