"""
Поиск и загрузка динамической библиотеки «Берёсты» через ctypes.

Порядок поиска (первый найденный файл побеждает):
  1. переменная окружения BERESTA_LIBRARY — явный путь к файлу;
  2. папка пакета _native/<os>-<arch>/ — туда бинарник кладут
     CMake (-DBERESTA_BUILD_PYTHON=ON) и Makefile (make PYTHON=1);
  3. системный поиск (ctypes.util.find_library) — библиотека,
     установленная в систему (/usr/lib, PATH и т.п.).

Имена ОС и архитектуры совпадают с именами в CMake, Makefile и
Java-обёртке: windows/linux/macos и x86_64/aarch64/x86.
"""

from __future__ import annotations

import ctypes
import ctypes.util
import os
import platform
import struct
import sys
from pathlib import Path

# Каноническое имя файла библиотеки на каждой ОС
_LIBRARY_FILES = {
    "windows": "beresta.dll",
    "linux": "libberesta.so",
    "macos": "libberesta.dylib",
}

# Разные системы называют одну архитектуру по-разному
_ARCH_ALIASES = {
    "amd64": "x86_64", "x86_64": "x86_64", "x64": "x86_64",
    "arm64": "aarch64", "aarch64": "aarch64",
    "i386": "x86", "i686": "x86", "x86": "x86",
}


def platform_name() -> str:
    """Имя платформы вида windows-x86_64 (имя папки в _native)."""
    if sys.platform.startswith("win"):
        system = "windows"
    elif sys.platform == "darwin":
        system = "macos"
    else:
        system = sys.platform.rstrip("0123456789")
    machine = platform.machine().lower()
    arch = _ARCH_ALIASES.get(machine, machine)
    # 32-битный Python на 64-битном процессоре загрузит только 32-битную библиотеку
    if arch == "x86_64" and struct.calcsize("P") == 4:
        arch = "x86"
    return system + "-" + arch


def library_file_name() -> str:
    """Имя файла библиотеки для текущей ОС."""
    system = platform_name().split("-")[0]
    return _LIBRARY_FILES.get(system, "libberesta.so")


def _candidates() -> list:
    """Кандидаты в порядке приоритета (несуществующие отбрасываются позже)."""
    result = []
    explicit = os.environ.get("BERESTA_LIBRARY")
    if explicit:
        result.append(explicit)
    bundled = Path(__file__).resolve().parent / "_native" / platform_name() / library_file_name()
    result.append(str(bundled))
    system_wide = ctypes.util.find_library("beresta")
    if system_wide:
        result.append(system_wide)
    return result


def load_library() -> ctypes.CDLL:
    """Загрузка библиотеки; понятная ошибка, если бинарник не найден."""
    tried = []
    for candidate in _candidates():
        if os.path.isabs(candidate) and not os.path.exists(candidate):
            tried.append(candidate)
            continue
        return ctypes.CDLL(candidate)
    raise OSError(
        "Берёста: динамическая библиотека не найдена для платформы "
        + platform_name() + ". Соберите её с флагом PYTHON=1 (make) или "
        + "-DBERESTA_BUILD_PYTHON=ON (CMake) либо задайте BERESTA_LIBRARY. "
        + "Проверенные пути: " + ", ".join(tried))
