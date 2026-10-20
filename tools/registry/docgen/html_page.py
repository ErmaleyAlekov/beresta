"""Шаблон HTML-страницы документа: шапка с названием продукта и версией, оглавление, тело.
Стили — в tools/registry/doc.css: одни и те же для страницы сайта и для печати в PDF
(правило @page задаёт A4, поля и номер страницы внизу).

The HTML page template of a document: a header with the product name and version, the table of
contents, the body. The styles are in tools/registry/doc.css: the same for the site page and for
printing to PDF (the @page rule sets A4, the margins and the page number at the bottom).
"""

from __future__ import annotations

import html

TEMPLATE = """<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title} — «Берёста» {version}</title>
<style>
{css}
</style>
</head>
<body>
<header class="doc-head">
<p class="doc-product">Программа для ЭВМ «XML-библиотека „Берёста“ (beresta)», версия {version}</p>
<p class="doc-nav"><a href="../index.html">Сайт продукта</a> · <a href="../documentation.html">Документация</a></p>
</header>
<main>
{toc}
{body}
</main>
</body>
</html>
"""


def _toc(entries: list) -> str:
    """Оглавление из заголовков второго уровня (пусто, если их меньше трёх).
    The table of contents from the second-level headings (empty if there are fewer than three)."""
    if len(entries) < 3:
        return ""
    items = "".join('<li><a href="#%s">%s</a></li>' % (anchor, text) for anchor, text in entries)
    return '<nav class="doc-toc"><p>Содержание</p><ol>%s</ol></nav>' % items


def build_page(outline: dict, version: str, css: str) -> str:
    """Готовая страница из разобранного документа (название, оглавление, тело).
    A ready page from a parsed document (the title, the table of contents, the body)."""
    title = html.unescape(outline["title"])
    # Оглавление встаёт после заголовка и вводных абзацев — перед первым разделом h2
    # The table of contents goes after the title and the opening paragraphs — before the first h2 section
    head, mark, rest = outline["html"].partition("<h2")
    body = head + _toc(outline["toc"]) + "\n" + mark + rest
    return TEMPLATE.format(title=html.escape(title), version=html.escape(version), css=css,
                           toc="", body=body)
