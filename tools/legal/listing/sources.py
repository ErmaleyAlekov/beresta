"""Сбор исходного текста программы: какие файлы и в каком порядке входят в листинг.

Порядок выбран так, чтобы и начало, и конец листинга сразу узнавались как «Берёста»:
сначала общий заголовок beresta.hpp и Си-интерфейс beresta_c.h, затем модули от нижних
слоёв к верхним (утилиты, кодировки, ядро DOM, парсер, DTD, XSD, XPath...), в каждом модуле —
заголовки, потом реализация. Последним идёт модуль capi — реализация Си-интерфейса, поэтому
последние страницы листинга — это внешняя граница библиотеки.

Collecting the program source text: which files go into the listing and in what order.
The order is chosen so that both the beginning and the end of the listing are recognized as
Beresta at once: first the umbrella header beresta.hpp and the C interface beresta_c.h, then the
modules from the lower layers to the upper ones (utilities, encodings, the DOM core, the parser,
DTD, XSD, XPath...), in every module the headers first, then the implementation. The capi module,
the C interface implementation, comes last, so the last listing pages are the library's outer boundary.
"""

from __future__ import annotations

import re
from pathlib import Path

from .errors import ListingError

# Модули от нижних слоёв к верхним; неизвестные модули встают перед capi по алфавиту
# The modules from the lower layers to the upper ones; unknown modules go before capi alphabetically
MODULE_ORDER = (
    "utils", "exception", "i18n", "encoding", "io", "core", "parser", "sax", "dtd",
    "content", "xsd", "xpath", "traversal", "serializer", "xinclude", "capi",
)

# Файлы, с которых начинается листинг / The files the listing starts with
LEADING_FILES = ("include/beresta.hpp", "include/capi/beresta_c.h")

# Расширения исходного текста / The source text extensions
SOURCE_SUFFIXES = (".hpp", ".h", ".cpp", ".c")


def _module_files(root: Path, module: str) -> list:
    """Файлы модуля: сначала include/<модуль>, затем src/<модуль>, каждый каталог по алфавиту.
    The module files: include/<module> first, then src/<module>, every directory alphabetically."""
    found = []
    for top in ("include", "src"):
        base = root / top / module
        if base.is_dir():
            # Сортировка по пути с «/» даёт одинаковый порядок на всех ОС
            # Sorting by the path with "/" gives the same order on all OSes
            paths = [p for p in base.rglob("*") if p.suffix in SOURCE_SUFFIXES and p.is_file()]
            found.extend(sorted(p.relative_to(root).as_posix() for p in paths))
    return found


def _module_names(root: Path) -> list:
    """Модули в порядке листинга: известные по MODULE_ORDER, новые — перед capi.
    The modules in listing order: the known ones by MODULE_ORDER, new ones before capi."""
    present = set()
    for top in ("include", "src"):
        if (root / top).is_dir():
            present.update(p.name for p in (root / top).iterdir() if p.is_dir())
    known = [m for m in MODULE_ORDER if m in present and m != "capi"]
    unknown = sorted(present.difference(MODULE_ORDER))
    tail = []
    if "capi" in present:
        tail.append("capi")
    return known + unknown + tail


def collect_files(root: Path) -> list:
    """Все файлы листинга (пути относительно корня, через «/») в порядке листинга.
    All listing files (paths relative to the root, with "/") in listing order."""
    ordered = [name for name in LEADING_FILES if (root / name).is_file()]
    # Файлы прямо в include/ и src/ (кроме ведущих) — сразу после ведущих
    # Files right in include/ and src/ (except the leading ones) go right after the leading ones
    for top in ("include", "src"):
        if (root / top).is_dir():
            loose = [p.relative_to(root).as_posix() for p in (root / top).iterdir()
                     if p.is_file() and p.suffix in SOURCE_SUFFIXES]
            ordered.extend(sorted(name for name in loose if name not in ordered))
    for module in _module_names(root):
        ordered.extend(name for name in _module_files(root, module) if name not in ordered)
    if not ordered:
        raise ListingError("no_sources", root)
    return ordered


def read_source_lines(root: Path, name: str) -> list:
    """Строки файла: UTF-8, переводы строк любой ОС, табуляция — 4 пробела.
    The file lines: UTF-8, line breaks of any OS, a tab becomes 4 spaces."""
    data = (root / name).read_bytes().decode("utf-8", errors="replace")
    if data.startswith("﻿"):
        data = data[1:]
    data = data.replace("\t", "    ")
    # Управляющие символы (кроме переводов строк) показываются как \xNN, иначе они невидимы
    # Control characters (except line breaks) are shown as \xNN, otherwise they are invisible
    for code in [c for c in range(32) if c not in (10, 13)] + [127]:
        data = data.replace(chr(code), "\\x%02X" % code)
    return data.splitlines()


def read_version(root: Path) -> str:
    """Версия программы из BERESTA_VERSION_STRING в beresta_c.h.
    The program version from BERESTA_VERSION_STRING in beresta_c.h."""
    header = root / "include" / "capi" / "beresta_c.h"
    match = None
    if header.is_file():
        content = header.read_text(encoding="utf-8", errors="replace")
        match = re.search(r'#define\s+BERESTA_VERSION_STRING\s+"([^"]+)"', content)
    if match is None:
        raise ListingError("no_version", header)
    return match.group(1)
