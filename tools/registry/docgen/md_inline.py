"""Строчная разметка Markdown: `код`, **полужирный**, *курсив*, [ссылка](адрес).

Сначала весь текст экранируется для HTML, потом участки кода прячутся за метками, чтобы
внутри них не срабатывала остальная разметка, и в конце возвращаются на место.

Inline Markdown markup: `code`, **bold**, *italic*, [link](address).
First the whole text is escaped for HTML, then the code spans are hidden behind marks so that
the rest of the markup does not fire inside them, and in the end they are put back.
"""

from __future__ import annotations

import html
import re

CODE_SPAN = re.compile(r"`([^`]+)`")
LINK = re.compile(r"\[([^\]]+)\]\(([^)\s]+)\)")
BOLD = re.compile(r"\*\*(.+?)\*\*")
ITALIC = re.compile(r"(?<![\w*])\*([^*\s][^*]*?)\*(?![\w*])")


def _link(match: re.Match) -> str:
    """Ссылка: адрес .md внутри документации меняется на .html собранной страницы.
    A link: an .md address inside the documentation becomes the .html of the built page."""
    target = match.group(2)
    if not target.startswith(("http://", "https://", "mailto:")) and ".md" in target:
        target = target.replace(".md", ".html")
    return '<a href="%s">%s</a>' % (target, match.group(1))


def render_inline(text: str) -> str:
    """Строка Markdown -> HTML (без блочных элементов).
    A Markdown line -> HTML (without block elements)."""
    spans = []

    def hide(match: re.Match) -> str:
        # Замыкание здесь проще, чем класс: метка — номер участка в списке spans
        # A closure is simpler than a class here: the mark is the span number in the spans list
        spans.append("<code>" + match.group(1) + "</code>")
        return "\x00%d\x00" % (len(spans) - 1)

    result = CODE_SPAN.sub(hide, html.escape(text, quote=False))
    result = LINK.sub(_link, result)
    result = BOLD.sub(r"<strong>\1</strong>", result)
    result = ITALIC.sub(r"<em>\1</em>", result)
    for index, span in enumerate(spans):
        result = result.replace("\x00%d\x00" % index, span)
    return result
