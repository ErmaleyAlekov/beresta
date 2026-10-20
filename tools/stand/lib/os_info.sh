# ============================================================================
#  Файл: lib/os_info.sh
#  Описание: Название и версия ОС для отчёта стенда «Берёсты».
# ============================================================================
#  File: lib/os_info.sh
#  Description: The OS name and version for the «Beresta» stand report.
# ============================================================================

# Печатает название ОС с версией. PRETTY_NAME не всегда содержит версию
# («Astra Linux», «ALT Container»), поэтому к нему добавляется VERSION_ID, а для
# Astra Linux — точный номер выпуска из /etc/astra_version (например, 1.7.11).
# os-release читается в подоболочке, чтобы не менять переменные стенда.
# Prints the OS name with its version. PRETTY_NAME does not always contain the
# version («Astra Linux», «ALT Container»), so VERSION_ID is added to it, and for
# Astra Linux — the exact release number from /etc/astra_version (e.g. 1.7.11).
# os-release is read in a subshell so as not to change the stand variables.
env_os_name() {
    name="$( (. /etc/os-release && printf '%s|%s' "$PRETTY_NAME" "$VERSION_ID") 2>/dev/null)"
    pretty="${name%%|*}"
    version="${name#*|}"
    if [ -z "$pretty" ]; then
        uname -s
        return 0
    fi
    # Версия дописывается, только если её ещё нет в названии
    # The version is appended only if the name does not contain it yet
    case "$pretty" in
        *"$version"*) ;;
        *) pretty="$pretty $version" ;;
    esac
    if [ -r /etc/astra_version ]; then
        pretty="$pretty ($(head -n 1 /etc/astra_version))"
    fi
    printf '%s\n' "$pretty"
}
