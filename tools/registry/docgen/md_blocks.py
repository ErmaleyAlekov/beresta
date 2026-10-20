"""Разбор документа Markdown на блоки и сборка HTML (паттерн Interpreter для подмножества Markdown).

Главный цикл смотрит на первую строку очередного блока и передаёт его нужной функции из
md_kinds; заголовки получают якоря в стиле GitHub, чтобы ссылки вида #раздел из
исходных .md работали и в HTML, а заголовки второго уровня попадают в оглавление.

Parsing a Markdown document into blocks and assembling HTML (the Interpreter pattern for a
Markdown subset). The main loop looks at the first line of the next block and hands the block to
the right md_kinds function; headings get GitHub-style anchors so that #section links from the
source .md files work in HTML too, and second-level headings go into the table of contents.
"""

from __future__ import annotations

import re

from .md_inline import render_inline
from .md_kinds import code_block, list_block, paragraph, starts_block, table

HEADING = re.compile(r"^(#{1,4})\s+(.*)$")


def slugify(title: str) -> str:
    """Якорь заголовка как у GitHub: строчные буквы, пробелы -> «-», без знаков препинания.
    A heading anchor as on GitHub: lower case, spaces -> "-", no punctuation."""
    text = re.sub(r"[`*]", "", title.strip().lower())
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


def _heading(match: re.Match, outline: dict) -> str:
    """Заголовок h1–h4 с якорем; h1 становится названием документа, h2 — пунктом оглавления.
    An h1–h4 heading with an anchor; h1 becomes the document title, h2 a table of contents entry."""
    level = len(match.group(1))
    title = match.group(2).strip()
    anchor = slugify(title)
    if level == 1 and not outline["title"]:
        outline["title"] = title
    if level == 2:
        outline["toc"].append((anchor, render_inline(title)))
    return '<h%d id="%s">%s</h%d>' % (level, anchor, render_inline(title), level)


def _block(lines: list, index: int, outline: dict) -> tuple:
    """Один блок, начиная со строки index: HTML и номер следующей строки.
    One block starting at the line index: HTML and the number of the next line."""
    line = lines[index]
    stripped = line.strip()
    match = HEADING.match(stripped)
    if match:
        return _heading(match, outline), index + 1
    if stripped.startswith("```"):
        return code_block(lines, index)
    if stripped.startswith("|"):
        return table(lines, index)
    if stripped == "---":
        return "<hr>", index + 1
    if starts_block(line) and not stripped.startswith(">"):
        return list_block(lines, index)
    return paragraph(lines, index)


def render_markdown(text: str) -> dict:
    """Документ Markdown -> {"title": название, "toc": [(якорь, текст)], "html": тело}.
    A Markdown document -> {"title": the title, "toc": [(anchor, text)], "html": the body}."""
    lines = text.replace("\r\n", "\n").split("\n")
    outline = {"title": "", "toc": [], "html": []}
    index = 0
    while index < len(lines):
        if not lines[index].strip():
            index += 1
            continue
        chunk, index = _block(lines, index, outline)
        outline["html"].append(chunk)
    outline["html"] = "\n".join(outline["html"])
    return outline
