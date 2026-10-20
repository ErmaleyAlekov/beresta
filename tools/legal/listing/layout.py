"""Раскладка листинга по страницам: перенос длинных строк, разбиение на страницы,
выбор первых и последних страниц.

На странице всегда одинаковое число строк, поэтому номер страницы в полном листинге
однозначно определяется исходным текстом и не зависит от программы просмотра PDF.

Laying the listing out into pages: wrapping long lines, splitting into pages,
choosing the first and the last pages.
A page always holds the same number of lines, so a page number in the full listing is
determined by the source text alone and does not depend on the PDF viewer.
"""

from __future__ import annotations

from pathlib import Path

from .messages import document_text
from .sources import read_source_lines


def wrap_line(line: str, width: int) -> list:
    """Строка, разрезанная на куски не длиннее width символов (пустая строка — один кусок).
    A line cut into pieces of at most width characters (an empty line is one piece)."""
    line = line.rstrip()
    if line == "":
        return [""]
    return [line[start:start + width] for start in range(0, len(line), width)]


def build_lines(root: Path, files: list, width: int) -> tuple:
    """Все строки листинга как пары (текст, файл) и сводка: файлов, строк, байт исходника.
    All listing lines as (text, file) pairs and a summary: files, lines, source bytes."""
    lines = []
    stats = {"files": len(files), "lines": 0, "bytes": 0}
    for name in files:
        # Строка-разделитель с путём файла открывает каждый файл
        # A separator line with the file path opens every file
        lines.append((document_text("file_mark", name), name))
        source = read_source_lines(root, name)
        stats["lines"] += len(source)
        stats["bytes"] += (root / name).stat().st_size
        for line in source:
            lines.extend((piece, name) for piece in wrap_line(line, width))
        lines.append(("", name))
    return lines, stats


def paginate(lines: list, per_page: int) -> list:
    """Страницы листинга: номер (с 1), файл первой строки и сами строки.
    The listing pages: the number (from 1), the file of the first line and the lines themselves."""
    pages = []
    for start in range(0, len(lines), per_page):
        chunk = lines[start:start + per_page]
        pages.append({
            "number": len(pages) + 1,
            "file": chunk[0][1],
            "lines": [entry[0] for entry in chunk],
        })
    return pages


def select_pages(pages: list, head: int, tail: int) -> list:
    """Первые head и последние tail страниц; короткий листинг выводится целиком.
    The first head and the last tail pages; a short listing is output whole."""
    if head < 0 or tail < 0 or len(pages) <= head + tail:
        return pages
    return pages[:head] + pages[len(pages) - tail:]


def wrap_words(paragraph: str, width: int) -> list:
    """Абзац, перенесённый по словам в строки не длиннее width (для текстовых документов).
    A paragraph wrapped by words into lines of at most width (for text documents)."""
    result = []
    current = ""
    for word in paragraph.split():
        if current == "":
            current = word
        elif len(current) + 1 + len(word) <= width:
            current = current + " " + word
        else:
            result.append(current)
            current = word
    if current != "" or not result:
        result.append(current)
    return result
