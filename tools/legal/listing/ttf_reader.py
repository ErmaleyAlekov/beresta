"""Чтение шрифта TrueType: только то, что нужно для встраивания в PDF.

Шрифт встраивается в PDF целиком (без выделения подмножества глифов): так файл проще
проверить, а требованию PDF/A «все шрифты встроены» он удовлетворяет. Из шрифта читаются
метрики (размер em, высоты, рамка), ширины глифов и таблица cmap формата 4 —
соответствие символов Unicode номерам глифов.

Reading a TrueType font: only what is needed to embed it into a PDF.
The font is embedded whole (without subsetting the glyphs): such a file is easier to check,
and it meets the PDF/A requirement "all fonts are embedded". The font gives its metrics
(the em size, the heights, the bounding box), the glyph widths and the format 4 cmap table —
the mapping of Unicode characters to glyph numbers.
"""

from __future__ import annotations

import struct
from pathlib import Path

from .errors import ListingError

# Таблицы, без которых шрифт не встроить / The tables without which the font cannot be embedded
REQUIRED_TABLES = ("head", "hhea", "hmtx", "maxp", "cmap", "OS/2", "post")


def read_tables(data: bytes, path: Path) -> dict:
    """Каталог таблиц шрифта: имя -> (смещение, длина); проверяет обязательные таблицы.
    The font table directory: name -> (offset, length); checks the required tables."""
    version, count = struct.unpack_from(">IH", data, 0)
    if version not in (0x00010000, 0x74727565):
        raise ListingError("bad_font", path, "sfnt")
    tables = {}
    for index in range(count):
        tag, _checksum, offset, length = struct.unpack_from(">4sIII", data, 12 + 16 * index)
        tables[tag.decode("latin-1")] = (offset, length)
    for name in REQUIRED_TABLES:
        if name not in tables:
            raise ListingError("bad_font", path, ListingError("font_no_table", name))
    return tables


def read_metrics(data: bytes, tables: dict) -> dict:
    """Метрики шрифта в единицах em: размер em, рамка, подъём, спуск, высота прописных, наклон.
    The font metrics in em units: the em size, the box, the ascent, the descent, the cap height, the slant."""
    head = tables["head"][0]
    hhea = tables["hhea"][0]
    os2, os2_length = tables["OS/2"]
    metrics = {
        "units": struct.unpack_from(">H", data, head + 18)[0],
        "bbox": struct.unpack_from(">hhhh", data, head + 36),
        "ascent": struct.unpack_from(">h", data, hhea + 4)[0],
        "descent": struct.unpack_from(">h", data, hhea + 6)[0],
        "hmetrics": struct.unpack_from(">H", data, hhea + 34)[0],
        "glyphs": struct.unpack_from(">H", data, tables["maxp"][0] + 4)[0],
        "fs_type": struct.unpack_from(">H", data, os2 + 8)[0],
        "weight": struct.unpack_from(">H", data, os2 + 4)[0],
        "italic": struct.unpack_from(">i", data, tables["post"][0] + 4)[0] / 65536.0,
    }
    # Высота прописных есть только в OS/2 версии 2 и новее; иначе — 70 % подъёма
    # The cap height exists only in OS/2 version 2 and newer; otherwise 70 % of the ascent
    metrics["cap"] = int(metrics["ascent"] * 0.7)
    if os2_length >= 90 and struct.unpack_from(">H", data, os2)[0] >= 2:
        metrics["cap"] = struct.unpack_from(">h", data, os2 + 88)[0]
    return metrics


def read_advances(data: bytes, tables: dict, metrics: dict) -> list:
    """Ширина каждого глифа в единицах em (хвост hmtx повторяет последнюю ширину).
    The width of every glyph in em units (the hmtx tail repeats the last width)."""
    offset = tables["hmtx"][0]
    widths = [struct.unpack_from(">H", data, offset + 4 * i)[0] for i in range(metrics["hmetrics"])]
    while len(widths) < metrics["glyphs"]:
        widths.append(widths[-1])
    return widths


def read_cmap(data: bytes, tables: dict, path: Path) -> dict:
    """Соответствие «символ Unicode -> номер глифа» из подтаблицы cmap формата 4 (Windows Unicode).
    The "Unicode character -> glyph number" mapping from the format 4 cmap subtable (Windows Unicode)."""
    base = tables["cmap"][0]
    sub = None
    for index in range(struct.unpack_from(">H", data, base + 2)[0]):
        platform, encoding, offset = struct.unpack_from(">HHI", data, base + 4 + 8 * index)
        if (platform, encoding) in ((3, 1), (0, 3)) and struct.unpack_from(">H", data, base + offset)[0] == 4:
            sub = base + offset
    if sub is None:
        raise ListingError("bad_font", path, ListingError("font_no_cmap"))
    segments = struct.unpack_from(">H", data, sub + 6)[0] // 2
    ends, starts = sub + 14, sub + 16 + 2 * segments
    deltas, ranges = starts + 2 * segments, starts + 4 * segments
    mapping = {}
    for seg in range(segments):
        end, start = struct.unpack_from(">H", data, ends + 2 * seg)[0], struct.unpack_from(">H", data, starts + 2 * seg)[0]
        delta, shift = struct.unpack_from(">h", data, deltas + 2 * seg)[0], struct.unpack_from(">H", data, ranges + 2 * seg)[0]
        for code in range(start, min(end, 0xFFFE) + 1):
            glyph = code
            if shift != 0:
                # Номер берётся из glyphIdArray по смещению от самой ячейки idRangeOffset
                # The number is taken from glyphIdArray at an offset from the idRangeOffset cell itself
                glyph = struct.unpack_from(">H", data, ranges + 2 * seg + shift + 2 * (code - start))[0]
            if glyph != 0:
                mapping[code] = (glyph + delta) & 0xFFFF
    return mapping


def load_font(path: Path) -> dict:
    """Шрифт целиком: байты файла, метрики, ширины глифов и cmap; проверяет право встраивания.
    The whole font: the file bytes, the metrics, the glyph widths and the cmap; checks the embedding right."""
    data = path.read_bytes()
    tables = read_tables(data, path)
    metrics = read_metrics(data, tables)
    # fsType == 2 — «ограниченная лицензия»: встраивать шрифт нельзя
    # fsType == 2 means a "restricted license": the font must not be embedded
    if metrics["fs_type"] & 0x000F == 0x0002:
        raise ListingError("bad_font", path, ListingError("font_restricted"))
    return {
        "path": path,
        "data": data,
        "metrics": metrics,
        "widths": read_advances(data, tables, metrics),
        "cmap": read_cmap(data, tables, path),
    }
