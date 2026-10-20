"""Сборка документа PDF целиком: титульный лист, страницы, шрифт, метаданные, каталог.

Два вида документа: листинг (титульный лист + выбранные страницы листинга с двойной
нумерацией) и простой текст (реферат: абзацы с переносом по словам, полуторный интервал).

Assembling the whole PDF document: the title page, the pages, the font, the metadata, the catalog.
Two kinds of document: the listing (the title page + the chosen listing pages with double
numbering) and plain text (the abstract: paragraphs wrapped by words, one-and-a-half spacing).
"""

from __future__ import annotations

import hashlib
from pathlib import Path

from .layout import wrap_words
from .messages import document_text
from .pdf_font import write_font
from .pdf_metadata import write_metadata
from .pdf_pages import body_page, page_geometry, title_page
from .pdf_writer import PdfWriter


def _add_page(writer: PdfWriter, numbers: dict, content: bytes) -> int:
    """Объект страницы A4 со своим потоком содержимого и шрифтом F1.
    An A4 page object with its own content stream and the F1 font."""
    stream = writer.add(writer.stream("", content))
    return writer.add((
        "<< /Type /Page /Parent %d 0 R /MediaBox [0 0 595.28 841.89]"
        " /Resources << /Font << /F1 %d 0 R >> >> /Contents %d 0 R >>"
        % (numbers["tree"], numbers["font"], stream)).encode("ascii"))


def _finish(writer: PdfWriter, numbers: dict, context: dict, pages: list) -> None:
    """Дерево страниц, шрифт, метаданные и каталог; запись файла с идентификатором из содержимого.
    The page tree, the font, the metadata and the catalog; writes the file with an id taken from the content."""
    kids = " ".join("%d 0 R" % page for page in pages)
    writer.put(numbers["tree"], ("<< /Type /Pages /Kids [%s] /Count %d >>" % (kids, len(pages))).encode("ascii"))
    write_font(writer, context["font"], numbers["font"])
    meta = write_metadata(writer, context["title"], context["moment"], context["icc"])
    intent = ""
    if meta["intent"] is not None:
        intent = " /OutputIntents [%d 0 R]" % meta["intent"]
    root = writer.add(("<< /Type /Catalog /Pages %d 0 R /Metadata %d 0 R%s /Lang (ru-RU) >>"
                       % (numbers["tree"], meta["metadata"], intent)).encode("ascii"))
    # Идентификатор — хеш объектов: одинаковые входные данные дают одинаковый файл
    # The id is a hash of the objects: equal input data give an equal file
    digest = hashlib.md5(b"".join(writer.objects)).hexdigest().upper()
    writer.write(context["output"], root, meta["info"], digest)


def _title_blocks(context: dict) -> list:
    """Строки титульного листа: (текст, кегль, отступ сверху в пунктах).
    The title page lines: (text, size, space above in points)."""
    stats, total, head = context["stats"], context["total"], context["head"]
    blocks = [(document_text("title_kind"), 14, 0), (document_text("title_name"), 22, 30),
              (document_text("title_version", context["version"]), 14, 24),
              (document_text("title_listing"), 14, 40)]
    for line in wrap_words(document_text("title_holder", context["holder"]), 70):
        blocks.append((line, 11, 18))
    for line in wrap_words(document_text("title_authors", context["authors"]), 70):
        blocks.append((line, 11, 18))
    pages = document_text("title_pages_all", total)
    if head + context["tail"] < total:
        pages = document_text("title_pages", head, total - context["tail"] + 1, total)
    notes = [document_text("title_language"),
             document_text("title_volume", stats["files"], stats["lines"], stats["bytes"]),
             pages, document_text("title_numbers")]
    for note in notes:
        blocks.append((note, 10, 18))
    blocks.append((document_text("title_year", context["moment"].year), 12, 120))
    return blocks


def write_listing(context: dict) -> int:
    """Файл листинга: титульный лист и выбранные страницы; возвращает число страниц файла.
    The listing file: the title page and the chosen pages; returns the number of pages in the file."""
    writer = PdfWriter()
    font = context["font"]
    numbers = {"tree": writer.add(), "font": writer.add()}
    geometry = page_geometry(font, 10, 14, 50)
    pages = [_add_page(writer, numbers, title_page(font, geometry, _title_blocks(context)))]
    header = document_text("header", context["version"])
    for sequence, page in enumerate(context["pages"], start=1):
        footer = document_text("footer", sequence, page["number"], context["total"])
        content = body_page(font, geometry, (header, page["file"], footer), page["lines"])
        pages.append(_add_page(writer, numbers, content))
    _finish(writer, numbers, context, pages)
    return len(pages)


def write_text(context: dict, paragraphs: list) -> int:
    """Файл простого текста: абзацы с переносом по словам, 12 пт, полуторный интервал.
    A plain text file: paragraphs wrapped by words, 12 pt, one-and-a-half spacing."""
    writer = PdfWriter()
    font = context["font"]
    numbers = {"tree": writer.add(), "font": writer.add()}
    geometry = page_geometry(font, 12, 18, 38)
    lines = []
    for paragraph in paragraphs:
        lines.extend(wrap_words(paragraph, geometry["columns"]))
        lines.append("")
    chunks = [lines[i:i + geometry["lines"]] for i in range(0, len(lines), geometry["lines"])]
    pages = []
    for index, chunk in enumerate(chunks, start=1):
        footer = document_text("text_footer", index, len(chunks))
        pages.append(_add_page(writer, numbers, body_page(font, geometry, (context["title"], "", footer), chunk)))
    _finish(writer, numbers, context, pages)
    return len(pages)
