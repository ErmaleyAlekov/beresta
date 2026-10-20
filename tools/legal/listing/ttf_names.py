"""Имя шрифта PostScript из таблицы name — его ставят в /BaseFont встроенного шрифта.
The font PostScript name from the name table — it goes into /BaseFont of the embedded font."""

from __future__ import annotations

import re
import struct


def postscript_name(data: bytes, tables: dict) -> str:
    """Имя PostScript (nameID 6) или «BerestaMono», если таблицы name нет или имя пустое.
    The PostScript name (nameID 6) or "BerestaMono" if there is no name table or the name is empty."""
    if "name" not in tables:
        return "BerestaMono"
    base = tables["name"][0]
    count, strings = struct.unpack_from(">HH", data, base + 2)
    found = ""
    for index in range(count):
        platform, _encoding, _language, name_id, length, offset = struct.unpack_from(">6H", data, base + 6 + 12 * index)
        if name_id != 6:
            continue
        raw = data[base + strings + offset:base + strings + offset + length]
        # Платформа 3 (Windows) и 0 (Unicode) хранят имя в UTF-16BE, платформа 1 — в одном байте
        # Platform 3 (Windows) and 0 (Unicode) keep the name in UTF-16BE, platform 1 in single bytes
        if platform in (0, 3):
            found = raw.decode("utf-16-be", errors="ignore")
        elif found == "":
            found = raw.decode("latin-1")
    # В имени PDF допустимы только печатные символы без разделителей
    # A PDF name allows only printable characters without delimiters
    found = re.sub(r"[^A-Za-z0-9_.+-]", "", found)
    if found == "":
        return "BerestaMono"
    return found
