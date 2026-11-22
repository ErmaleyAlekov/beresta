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
from .ttf_cmap import read_cmap
from .ttf_names import postscript_name

# Таблицы, без которых шрифт не встроить / The tables without which the font cannot be embedded
REQUIRED_TABLES = ("head", "hhea", "hmtx", "maxp", "cmap", "OS/2", "post")


def read_tables(data: bytes, path: Path) -> dict:
    """Каталог таблиц шрифта: имя -> (смещение, длина); проверяет обязательные таблицы.
    The font table directory: name -> (offset, length); checks the required tables."""
    if len(data) < 12:
        raise ListingError("bad_font", path, "sfnt")
    version, count = struct.unpack_from(">IH", data, 0)
    if version not in (0x00010000, 0x74727565):
        raise ListingError("bad_font", path, "sfnt")
    if 12 + 16 * count > len(data):
        raise ListingError("bad_font", path, "sfnt")
    tables = {}
    for index in range(count):
        tag, _checksum, offset, length = struct.unpack_from(">4sIII", data, 12 + 16 * index)
        if offset + length > len(data):
            raise ListingError("bad_font", path, tag.decode("latin-1"))
        tables[tag.decode("latin-1")] = (offset, length)
    for name in REQUIRED_TABLES:
        if name not in tables:
            raise ListingError("bad_font", path, ListingError("font_no_table", name))
    return tables


def read_metrics(data: bytes, tables: dict) -> dict:
    """Метрики шрифта в единицах em: размер em, рамка, подъём, спуск, высота прописных, наклон.
    The font metrics in em units: the em size, the box, the ascent, the descent, the cap height, the slant."""
    def table(name):
        offset, length = tables[name]
        return data[offset:offset + length]

    head, hhea, os2 = table("head"), table("hhea"), table("OS/2")
    metrics = {
        "units": struct.unpack_from(">H", head, 18)[0],
        "bbox": struct.unpack_from(">hhhh", head, 36),
        "ascent": struct.unpack_from(">h", hhea, 4)[0],
        "descent": struct.unpack_from(">h", hhea, 6)[0],
        "hmetrics": struct.unpack_from(">H", hhea, 34)[0],
        "glyphs": struct.unpack_from(">H", table("maxp"), 4)[0],
        "fs_type": struct.unpack_from(">H", os2, 8)[0],
        "weight": struct.unpack_from(">H", os2, 4)[0],
        "italic": struct.unpack_from(">i", table("post"), 4)[0] / 65536.0,
    }
    # Высота прописных есть только в OS/2 версии 2 и новее; иначе — 70 % подъёма
    # The cap height exists only in OS/2 version 2 and newer; otherwise 70 % of the ascent
    metrics["cap"] = int(metrics["ascent"] * 0.7)
    if len(os2) >= 90 and struct.unpack_from(">H", os2)[0] >= 2:
        metrics["cap"] = struct.unpack_from(">h", os2, 88)[0]
    return metrics


def read_advances(data: bytes, tables: dict, metrics: dict) -> list:
    """Ширина каждого глифа в единицах em (хвост hmtx повторяет последнюю ширину).
    The width of every glyph in em units (the hmtx tail repeats the last width)."""
    offset, length = tables["hmtx"]
    hmetrics, glyphs = metrics["hmetrics"], metrics["glyphs"]
    if not 0 < hmetrics <= glyphs or 4 * hmetrics + 2 * (glyphs - hmetrics) > length:
        raise ListingError("font_invalid_table", "hmtx")
    widths = [struct.unpack_from(">H", data, offset + 4 * i)[0] for i in range(metrics["hmetrics"])]
    while len(widths) < metrics["glyphs"]:
        widths.append(widths[-1])
    return widths


def load_font(path: Path) -> dict:
    """Шрифт целиком: байты файла, метрики, ширины глифов и cmap; проверяет право встраивания.
    The whole font: the file bytes, the metrics, the glyph widths and the cmap; checks the embedding right."""
    data = path.read_bytes()
    tables = read_tables(data, path)
    try:
        metrics = read_metrics(data, tables)
        # Запрет встраивания или разрешение только для растровых глифов.
        # Embedding is restricted or permitted for bitmap glyphs only.
        if metrics["fs_type"] & 0x000F == 0x0002 or metrics["fs_type"] & 0x0200:
            raise ListingError("font_restricted")
        if metrics["units"] == 0:
            raise ListingError("font_invalid_table", "head")
        name = postscript_name(data, tables)
        widths = read_advances(data, tables, metrics)
        cmap = read_cmap(data, tables, metrics["glyphs"])
    except ListingError as error:
        raise ListingError("bad_font", path, error) from error
    except struct.error as error:
        raise ListingError("bad_font", path, ListingError("font_truncated")) from error
    return {"path": path, "data": data, "metrics": metrics, "name": name, "widths": widths, "cmap": cmap}
