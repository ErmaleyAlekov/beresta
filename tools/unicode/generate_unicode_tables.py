"""
Генератор таблиц Unicode 3.1 для регулярных выражений XSD 1.0.

Читает UnicodeData-3.1.0.txt и Blocks-4.txt (https://www.unicode.org/Public/3.1-Update/)
и пишет src/xsd/regex/unicode_category_table.cpp и unicode_block_table.cpp.
Нужен только при смене версии Unicode: готовые таблицы лежат в репозитории,
а сборка библиотеки Python не требует.

Запуск: python tools/unicode/generate_unicode_tables.py <папка с файлами UCD>

Generator of the Unicode 3.1 tables for XSD 1.0 regular expressions.

It reads UnicodeData-3.1.0.txt and Blocks-4.txt (https://www.unicode.org/Public/3.1-Update/)
and writes src/xsd/regex/unicode_category_table.cpp and unicode_block_table.cpp.
It is needed only when the Unicode version changes: the ready tables are in the repository,
and building the library does not require Python.

Usage: python tools/unicode/generate_unicode_tables.py <folder with the UCD files>
"""

import os
import sys

# Корень репозитория — две папки выше скрипта
# The repository root is two folders above the script
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT_DIR = os.path.join(ROOT, "src", "xsd", "regex")

# Блоки «Private Use» вне BMP таблица XSD 1.0 2-й редакции не включает в IsPrivateUse
# The XSD 1.0 2nd edition table does not include the "Private Use" blocks outside the BMP in IsPrivateUse
SKIPPED_BLOCKS = {(0xF0000, 0xFFFFD), (0x100000, 0x10FFFD)}

INVALID_RANGE = "{0}:{1}: invalid First>/Last> range in UnicodeData"


class UnicodeDataError(ValueError):
    """Несогласованные границы диапазона UnicodeData.
    Inconsistent boundaries of a UnicodeData range."""


def _category_ranges(source, path):
    """Разворачивает записи диапазонов и проверяет, что границы образуют пары.
    Expands range records and checks that their boundaries form pairs."""
    first = None
    for line_number, line in enumerate(source, 1):
        fields = line.strip().split(";")
        if len(fields) < 3:
            continue
        code = int(fields[0], 16)
        name, category = fields[1:3]
        if first is not None:
            start, first_name, first_category, _ = first
            if name != first_name or category != first_category or start >= code:
                raise UnicodeDataError(INVALID_RANGE.format(path, line_number))
            first = None
            yield start, code, category
        elif name.endswith(", First>"):
            first = code, name[:-len("First>")] + "Last>", category, line_number
        elif name.endswith("Last>"):
            raise UnicodeDataError(INVALID_RANGE.format(path, line_number))
        else:
            yield code, code, category
    if first is not None:
        raise UnicodeDataError(INVALID_RANGE.format(path, first[3]))


def load_categories(path):
    """
    Диапазоны одинаковой категории: соседние коды с той же категорией
    сливаются; пары <..., First>/<..., Last> задают сплошной диапазон.

    Ranges of the same category: adjacent codes with the same category
    are merged; the <..., First>/<..., Last> pairs define a solid range.
    """
    ranges = []
    with open(path, encoding="utf-8") as source:
        for start, code, category in _category_ranges(source, path):
            if ranges and ranges[-1][1] == start - 1 and ranges[-1][2] == category:
                ranges[-1][1] = code
            else:
                ranges.append([start, code, category])
    return ranges


def load_blocks(path):
    """
    Блоки: имя без пробелов (как в \\p{IsИмя}) и диапазон кодов.

    The blocks: the name without spaces (as in \\p{IsName}) and the code range.
    """
    blocks = []
    with open(path, encoding="utf-8") as source:
        for line in source:
            line = line.split("#")[0].strip()
            if not line:
                continue
            codes, name = line.split(";")
            first, last = (int(part, 16) for part in codes.split(".."))
            if (first, last) in SKIPPED_BLOCKS:
                continue
            blocks.append(("Is" + name.strip().replace(" ", ""), first, last))
    return blocks


def write_table(file_name, header, names, rows):
    """
    Файл .cpp с одним константным массивом и его длиной; по 4 элемента в строке.
    names — (тип элемента, имя массива, имя длины).

    A .cpp file with one constant array and its length; 4 items per line.
    names is (the item type, the array name, the length name).
    """
    item_type, array, count = names
    lines = ['#include "xsd/regex/unicode_tables.hpp"', "", "namespace beresta::xsd", "{"]
    lines += ["    " + text for text in header]
    lines.append("    const %s %s[] = {" % (item_type, array))
    for index in range(0, len(rows), 4):
        lines.append("        " + " ".join(rows[index:index + 4]))
    lines[-1] = lines[-1].rstrip(",")
    lines += ["    };", "", "    const std::size_t %s = sizeof(%s) / sizeof(%s[0]);" % (count, array, array), "}", ""]
    with open(os.path.join(OUT_DIR, file_name), "w", encoding="utf-8", newline="\n") as target:
        target.write("\n".join(lines))


def main():
    """
    Чтение двух файлов UCD и запись двух таблиц.

    Reads two UCD files and writes two tables.
    """
    if len(sys.argv) != 2:
        sys.exit("usage: generate_unicode_tables.py <UCD folder>")
    try:
        categories = load_categories(os.path.join(sys.argv[1], "UnicodeData-3.1.0.txt"))
    except UnicodeDataError as error:
        sys.exit(str(error))
    blocks = load_blocks(os.path.join(sys.argv[1], "Blocks-4.txt"))
    write_table("unicode_category_table.cpp", [
        "// Сгенерировано tools/unicode/generate_unicode_tables.py из UnicodeData-3.1.0.txt; не править вручную",
        "// Generated by tools/unicode/generate_unicode_tables.py from UnicodeData-3.1.0.txt; do not edit by hand",
        "// Диапазоны общих категорий Unicode 3.1: {первый, последний, класс, подкласс}",
        "// The ranges of the Unicode 3.1 general categories: {first, last, class, subclass}"],
        ("CategoryRange", "UNICODE_CATEGORIES", "UNICODE_CATEGORY_COUNT"),
        ["{0x%04X, 0x%04X, '%s', '%s'}," % (a, b, c[0], c[1]) for a, b, c in categories])
    write_table("unicode_block_table.cpp", [
        "// Сгенерировано tools/unicode/generate_unicode_tables.py из Blocks-4.txt; не править вручную",
        "// Generated by tools/unicode/generate_unicode_tables.py from Blocks-4.txt; do not edit by hand",
        "// Блоки Unicode 3.1; IsPrivateUse — только E000-F8FF, как в таблице XSD 1.0 2-й редакции",
        "// The Unicode 3.1 blocks; IsPrivateUse is only E000-F8FF, as in the XSD 1.0 2nd edition table"],
        ("BlockRange", "UNICODE_BLOCKS", "UNICODE_BLOCK_COUNT"),
        ['{"%s", 0x%04X, 0x%04X},' % block for block in blocks])


if __name__ == "__main__":
    main()
