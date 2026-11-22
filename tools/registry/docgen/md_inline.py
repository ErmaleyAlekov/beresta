"""Строчная разметка Markdown: `код`, **полужирный**, *курсив*, [ссылка](адрес).

Сначала текст экранируется для HTML. Затем код и ссылки заменяются метками, чтобы
полужирное начертание и курсив не меняли уже готовые HTML-фрагменты.

Inline Markdown markup: `code`, **bold**, *italic*, [link](address).
First the text is escaped for HTML. Code spans and links are then replaced by markers so that
bold and italic formatting cannot change the HTML fragments already rendered.
"""

from __future__ import annotations

import html
import re
from urllib.parse import urlsplit

CODE_SPAN = re.compile(r"`([^`]+)`")
LINK = re.compile(r"\[([^\]]+)\]\(([^)\s]+)\)")
BOLD = re.compile(r"\*\*(.+?)\*\*")
ITALIC = re.compile(r"(?<![\w*])\*([^*\s][^*]*?)\*(?![\w*])")


def _emphasis(text: str) -> str:
    """Применяет полужирное начертание и курсив.
    Applies bold and italic formatting."""
    text = BOLD.sub(r"<strong>\1</strong>", text)
    return ITALIC.sub(r"<em>\1</em>", text)


def _link(match: re.Match) -> str:
    """Разрешает локальные ссылки, HTTP(S) и почту, экранирует адрес для HTML.
    Allows local links, HTTP(S) and mail, and escapes the URL for HTML."""
    target = html.unescape(match.group(2))
    label = _emphasis(match.group(1))
    if any(ord(char) < 32 for char in target):
        return label
    try:
        address = urlsplit(target)
    except ValueError:
        return label
    if address.scheme and address.scheme not in ("http", "https", "mailto"):
        return label
    if not address.scheme and not address.netloc and address.path.endswith(".md"):
        target = address._replace(path=address.path[:-3] + ".html").geturl()
    return '<a href="%s">%s</a>' % (html.escape(target, quote=True), label)


def render_inline(text: str) -> str:
    """Строка Markdown -> HTML (без блочных элементов).
    A Markdown line -> HTML (without block elements)."""
    spans = []

    def hide(fragment: str) -> str:
        # Метка хранит индекс HTML-фрагмента, защищённого от повторной разметки.
        # The marker stores the index of the HTML fragment protected from further formatting.
        spans.append(fragment)
        return "\x00%d\x00" % (len(spans) - 1)

    escaped = html.escape(text.replace("\x00", "\ufffd"), quote=False)
    result = CODE_SPAN.sub(lambda match: hide("<code>" + match.group(1) + "</code>"), escaped)
    result = LINK.sub(lambda match: hide(_link(match)), result)
    result = _emphasis(result)
    for index in reversed(range(len(spans))):
        span = spans[index]
        result = result.replace("\x00%d\x00" % index, span)
    return result
