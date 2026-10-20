"""Встраивание шрифта TrueType в PDF как составного шрифта Type0 / CIDFontType2.

Почему составной шрифт: текст листинга двуязычный (кириллица и латиница), а простой шрифт
PDF вмещает только 256 символов. С кодировкой Identity-H код символа в потоке страницы —
это сразу номер глифа (2 байта), а таблица ToUnicode позволяет искать и копировать текст.

Embedding a TrueType font into a PDF as a composite Type0 / CIDFontType2 font.
Why a composite font: the listing text is bilingual (Cyrillic and Latin), while a simple PDF font
holds only 256 characters. With the Identity-H encoding a character code in the page stream is
the glyph number itself (2 bytes), and the ToUnicode table makes the text searchable and copyable.
"""

from __future__ import annotations

from .pdf_writer import PdfWriter
from .ttf_names import postscript_name
from .ttf_reader import read_tables


def encode_text(font: dict, text: str) -> str:
    """Строка как шестнадцатеричные номера глифов; символы без глифа заменяются на «?».
    A string as hexadecimal glyph numbers; characters without a glyph become "?"."""
    codes = []
    for char in text:
        glyph = font["cmap"].get(ord(char))
        if glyph is None:
            font["missing"] = font.get("missing", 0) + 1
            char = "?"
            glyph = font["cmap"].get(ord(char), 0)
        # Запоминаем глиф для таблиц ширин и ToUnicode
        # Remember the glyph for the width and ToUnicode tables
        font.setdefault("used", {})[glyph] = char
        codes.append("%04X" % glyph)
    return "<" + "".join(codes) + ">"


def text_width(font: dict, text: str, size: float) -> float:
    """Ширина строки в пунктах при размере шрифта size.
    The string width in points at the font size size."""
    units = font["metrics"]["units"]
    total = 0
    for char in text:
        glyph = font["cmap"].get(ord(char), font["cmap"].get(ord("?"), 0))
        total += font["widths"][glyph]
    return total * size / units


def _scaled(font: dict, value: float) -> int:
    """Величина в единицах em шрифта, пересчитанная в тысячные доли кегля (единицы PDF).
    A value in the font em units converted to thousandths of the size (PDF units)."""
    return int(round(value * 1000.0 / font["metrics"]["units"]))


def _to_unicode(font: dict) -> bytes:
    """Таблица CMap «номер глифа -> символ Unicode» для поиска и копирования текста.
    The "glyph number -> Unicode character" CMap for searching and copying the text."""
    pairs = sorted(font.get("used", {}).items())
    lines = ["/CIDInit /ProcSet findresource begin", "12 dict begin", "begincmap",
             "/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def",
             "/CMapName /Adobe-Identity-UCS def", "/CMapType 2 def",
             "1 begincodespacerange", "<0000> <FFFF>", "endcodespacerange"]
    # В одном блоке bfchar — не больше 100 записей / One bfchar block holds at most 100 entries
    for start in range(0, len(pairs), 100):
        block = pairs[start:start + 100]
        lines.append("%d beginbfchar" % len(block))
        for glyph, char in block:
            lines.append("<%04X> <%s>" % (glyph, char.encode("utf-16-be").hex().upper()))
        lines.append("endbfchar")
    lines += ["endcmap", "CMapName currentdict /CMap defineresource pop", "end", "end"]
    return "\n".join(lines).encode("ascii")


def write_font(writer: PdfWriter, font: dict, number: int) -> None:
    """Объекты шрифта (файл шрифта, описатель, CID-шрифт, ToUnicode) и Type0 в зарезервированный номер number.
    Пишется после страниц: только тогда известны все использованные глифы.
    The font objects (the font file, the descriptor, the CID font, ToUnicode) and Type0 into the reserved number.
    It is written after the pages: only then all the used glyphs are known."""
    metrics = font["metrics"]
    name = postscript_name(font["data"], read_tables(font["data"], font["path"]))
    program = writer.add(writer.stream("/Length1 %d" % len(font["data"]), font["data"]))
    box = " ".join(str(_scaled(font, value)) for value in metrics["bbox"])
    descriptor = writer.add((
        "<< /Type /FontDescriptor /FontName /%s /Flags 33 /FontBBox [%s] /ItalicAngle %s"
        " /Ascent %d /Descent %d /CapHeight %d /StemV 80 /FontFile2 %d 0 R >>"
        % (name, box, metrics["italic"], _scaled(font, metrics["ascent"]),
           _scaled(font, metrics["descent"]), _scaled(font, metrics["cap"]), program)).encode("ascii"))
    # Ширины только использованных глифов: формат «глиф [ширина]»
    # The widths of the used glyphs only: the "glyph [width]" form
    widths = " ".join("%d [%d]" % (g, _scaled(font, font["widths"][g])) for g in sorted(font.get("used", {})))
    cid_font = writer.add((
        "<< /Type /Font /Subtype /CIDFontType2 /BaseFont /%s /CIDSystemInfo << /Registry (Adobe)"
        " /Ordering (Identity) /Supplement 0 >> /FontDescriptor %d 0 R /DW %d /W [%s] /CIDToGIDMap /Identity >>"
        % (name, descriptor, _scaled(font, font["widths"][0]), widths)).encode("ascii"))
    unicode_map = writer.add(writer.stream("", _to_unicode(font)))
    writer.put(number, (
        "<< /Type /Font /Subtype /Type0 /BaseFont /%s /Encoding /Identity-H /DescendantFonts [%d 0 R]"
        " /ToUnicode %d 0 R >>" % (name, cid_font, unicode_map)).encode("ascii"))
