"""Виды блоков Markdown: код, таблица, список, абзац (и цитата).

Каждая функция получает все строки документа и номер первой строки блока, а возвращает
готовый HTML и номер строки сразу после блока. Поддерживается то подмножество Markdown,
которым написаны документы проекта: без вложенных списков и HTML внутри текста.

The Markdown block kinds: code, a table, a list, a paragraph (and a quote).
Every function gets all the document lines and the number of the block's first line, and returns
ready HTML and the number of the line right after the block. The supported subset of Markdown is
the one the project documents are written in: no nested lists and no HTML inside the text.
"""

from __future__ import annotations

import html
import re

from .md_inline import render_inline

LIST_ITEM = re.compile(r"^\s*([-*]|\d+\.)\s+(.*)$")


def starts_block(line: str) -> bool:
    """Начинает ли строка новый блок (заголовок, код, таблица, список, цитата, линия).
    Whether the line starts a new block (a heading, code, a table, a list, a quote, a rule)."""
    stripped = line.strip()
    if stripped.startswith(("#", "```", "|", ">")) or stripped == "---":
        return True
    return LIST_ITEM.match(line) is not None


def code_block(lines: list, index: int) -> tuple:
    """Блок кода между строками ``` (язык после ``` становится классом).
    A code block between ``` lines (the language after ``` becomes a class)."""
    language = lines[index].strip()[3:].strip()
    body = []
    index += 1
    while index < len(lines) and not lines[index].strip().startswith("```"):
        body.append(lines[index])
        index += 1
    attribute = ""
    if language:
        attribute = ' class="language-%s"' % html.escape(language)
    return "<pre><code%s>%s</code></pre>" % (attribute, html.escape("\n".join(body), quote=False)), index + 1


def table(lines: list, index: int) -> tuple:
    """Таблица из строк «| a | b |»; вторая строка (|---|) отделяет заголовок.
    A table of "| a | b |" lines; the second line (|---|) separates the header."""
    rows = []
    while index < len(lines) and lines[index].strip().startswith("|"):
        # Ячейки делятся по «|», кроме экранированного «\|»
        # The cells are split by "|", except the escaped "\|"
        cells = re.split(r"(?<!\\)\|", lines[index].strip().strip("|"))
        rows.append([render_inline(cell.strip().replace("\\|", "|")) for cell in cells])
        index += 1
    head = "".join("<th>%s</th>" % cell for cell in rows[0])
    body = ""
    for row in rows[2:]:
        body += "<tr>" + "".join("<td>%s</td>" % cell for cell in row) + "</tr>"
    return "<table><thead><tr>%s</tr></thead><tbody>%s</tbody></table>" % (head, body), index


def list_block(lines: list, index: int) -> tuple:
    """Маркированный или нумерованный список; строка с отступом продолжает пункт.
    A bulleted or numbered list; an indented line continues the item."""
    ordered = LIST_ITEM.match(lines[index]).group(1)[0].isdigit()
    items = []
    while index < len(lines) and lines[index].strip():
        match = LIST_ITEM.match(lines[index])
        if match:
            items.append(match.group(2))
        elif lines[index].startswith(" ") and items:
            items[-1] += " " + lines[index].strip()
        else:
            break
        index += 1
    tag = "ul"
    if ordered:
        tag = "ol"
    inner = "".join("<li>%s</li>" % render_inline(item) for item in items)
    return "<%s>%s</%s>" % (tag, inner, tag), index


def paragraph(lines: list, index: int) -> tuple:
    """Абзац до пустой строки или начала другого блока; строки с «>» — цитата.
    A paragraph up to an empty line or the start of another block; lines with ">" are a quote."""
    quote = lines[index].strip().startswith(">")
    text = []
    while index < len(lines) and lines[index].strip():
        line = lines[index].strip()
        if text and (starts_block(line) and not (quote and line.startswith(">"))):
            break
        text.append(line.lstrip(">").strip())
        index += 1
    result = "<p>%s</p>" % render_inline(" ".join(text))
    if quote:
        result = "<blockquote>%s</blockquote>" % result
    return result, index
