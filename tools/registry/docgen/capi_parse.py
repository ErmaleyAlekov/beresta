"""Разбор заголовка Си-интерфейса include/capi/beresta_c.h: функции по группам, коды состояния, флаги.

Справочник API в руководстве по эксплуатации строится из самого заголовка, а не пишется
вручную: так в документе не может оказаться функции, которой нет, а новая функция попадёт
в него при следующей сборке. Описание берётся из русской части двуязычного комментария.

Parsing the C interface header include/capi/beresta_c.h: functions by groups, status codes, flags.
The API reference in the operation manual is built from the header itself instead of being written
by hand: so the document cannot contain a function that does not exist, and a new function gets
into it on the next build. The description is taken from the Russian part of the bilingual comment.
"""

from __future__ import annotations

import re

CYRILLIC = re.compile(r"[А-Яа-яЁё]")
PROTOTYPE_NAME = re.compile(r"(\w+)\s*\(")
ENUM_ENTRY = re.compile(r"^\s+(BERESTA_[A-Z_]+)\s*=\s*(\d+),?\s*(?:/\*(.*?)\*/)?")
FLAG_DEFINE = re.compile(r"^#define\s+(BERESTA_(?:PARSE|WRITE)_[A-Z_]+)\s+(\S+(?:\s<<\s\d+\))?)\s*(.*)$")


def russian(text: str) -> str:
    """Русская часть комментария: строки с кириллицей; хвост « / English text» отрезается.
    The Russian part of a comment: the lines with Cyrillic; an " / English text" tail is cut off."""
    kept = []
    for line in text.split("\n"):
        line = re.sub(r"^\s*(/\*+|\*\*)|\*/\s*$", "", line).strip()
        if not CYRILLIC.search(line):
            continue
        # Отрезать с первого « / », после которого — английская фраза (без кириллицы, с пробелом)
        # Cut from the first " / " followed by an English phrase (no Cyrillic, with a space)
        for match in re.finditer(r" / ", line):
            tail = line[match.end():].strip()
            if not CYRILLIC.search(tail) and " " in tail:
                line = line[:match.start()]
                break
        kept.append(line.strip())
    return " ".join(kept)


def parse_functions(lines: list) -> list:
    """Группы функций: [(название группы, [(сигнатура, имя, описание)])] в порядке заголовка.
    The function groups: [(group title, [(signature, name, description)])] in header order."""
    groups, comment, index = [], "", 0
    while index < len(lines):
        line = lines[index]
        if line.startswith("**  ") and index > 0 and lines[index - 1].startswith("/* ===="):
            groups.append((line[4:].split(" / ")[0].strip(), []))
        elif line.startswith("/*") and not line.startswith("/* ===="):
            # Комментарий — до строки с «*/»; подряд идущие комментарии склеиваются
            # A comment lasts up to the line with "*/"; consecutive comments are joined
            start = index
            while "*/" not in lines[index]:
                index += 1
            comment = comment + "\n" + "\n".join(lines[start:index + 1])
        elif line.startswith("BERESTA_API") and groups:
            start = index
            while ";" not in lines[index]:
                index += 1
            signature = " ".join(" ".join(lines[start:index + 1]).replace("BERESTA_API", "").split()).rstrip(";")
            groups[-1][1].append((signature, PROTOTYPE_NAME.search(signature).group(1), russian(comment)))
        elif not line.strip():
            comment = ""
        index += 1
    return groups


def parse_codes(lines: list) -> list:
    """Коды состояния из enum beresta_status: [(имя, значение, описание)].
    The status codes from enum beresta_status: [(name, value, description)]."""
    codes, inside = [], False
    for index, line in enumerate(lines):
        if line.startswith("typedef enum beresta_status"):
            inside = True
        elif inside and line.startswith("}"):
            break
        elif inside:
            match = ENUM_ENTRY.match(line)
            if match:
                # Продолжение комментария на следующей строке (без нового кода) тоже русское
                # A comment continued on the next line (without a new code) is Russian too
                text = match.group(3) or ""
                if index + 1 < len(lines) and not ENUM_ENTRY.match(lines[index + 1]) and "/*" in lines[index + 1]:
                    text += "\n" + lines[index + 1]
                codes.append((match.group(1), match.group(2), russian(text)))
    return codes


def parse_flags(lines: list) -> list:
    """Флаги разбора и вывода (#define BERESTA_PARSE_* / BERESTA_WRITE_*): [(имя, значение, описание)].
    The parsing and output flags (#define BERESTA_PARSE_* / BERESTA_WRITE_*): [(name, value, description)]."""
    flags = []
    for index, line in enumerate(lines):
        match = FLAG_DEFINE.match(line)
        if not match:
            continue
        text = [match.group(3)]
        # Комментарий может продолжаться строками с отступом до следующего #define
        # The comment may continue with indented lines up to the next #define
        follow = index + 1
        while follow < len(lines) and lines[follow].startswith(" ") and lines[follow].strip():
            text.append(lines[follow])
            follow += 1
        flags.append((match.group(1), match.group(2), russian("\n".join(text))))
    return flags
