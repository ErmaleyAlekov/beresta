"""Чтение Unicode-подтаблицы cmap формата 4 TrueType.
Reads the TrueType format 4 Unicode cmap subtable."""

from __future__ import annotations

import struct

from .errors import ListingError


def _format4(cmap: bytes) -> bytes:
    """Последняя поддерживаемая Unicode-подтаблица в пределах cmap.
    The last supported Unicode subtable within cmap."""
    count = struct.unpack_from(">H", cmap, 2)[0]
    directory_end = 4 + 8 * count
    if directory_end > len(cmap):
        raise ListingError("font_invalid_table", "cmap")
    subtable = None
    for index in range(count):
        platform, encoding, offset = struct.unpack_from(">HHI", cmap, 4 + 8 * index)
        if (platform, encoding) not in ((3, 1), (0, 3)):
            continue
        if offset < directory_end:
            raise ListingError("font_invalid_table", "cmap")
        if struct.unpack_from(">H", cmap, offset)[0] == 4:
            length = struct.unpack_from(">H", cmap, offset + 2)[0]
            if offset + length > len(cmap):
                raise ListingError("font_invalid_table", "cmap")
            subtable = cmap[offset:offset + length]
    if subtable is None:
        raise ListingError("font_no_cmap")
    return subtable


def _segment_glyphs(subtable: bytes, start: int, end: int, range_cell: int, array_start: int):
    """Номера глифов до применения idDelta, с проверкой границ glyphIdArray.
    Glyph IDs before applying idDelta, with glyphIdArray bounds checked."""
    shift = struct.unpack_from(">H", subtable, range_cell)[0]
    if shift == 0:
        return range(start, end + 1)
    offset = range_cell + shift
    count = end - start + 1
    if shift % 2 or offset < array_start or offset + 2 * count > len(subtable):
        raise ListingError("font_invalid_table", "cmap")
    return struct.unpack_from(">%dH" % count, subtable, offset)


def read_cmap(data: bytes, tables: dict, glyph_count: int) -> dict:
    """Соответствие «символ Unicode -> номер глифа» из формата 4.
    The Unicode character to glyph ID mapping from format 4."""
    offset, length = tables["cmap"]
    subtable = _format4(data[offset:offset + length])
    segment_bytes = struct.unpack_from(">H", subtable, 6)[0]
    if not segment_bytes or segment_bytes % 2 or 16 + 4 * segment_bytes > len(subtable):
        raise ListingError("font_invalid_table", "cmap")
    starts = 16 + segment_bytes
    deltas, ranges = starts + segment_bytes, starts + 2 * segment_bytes
    mapping = {}
    previous_end = -1
    for cell in range(0, segment_bytes, 2):
        end = struct.unpack_from(">H", subtable, 14 + cell)[0]
        start = struct.unpack_from(">H", subtable, starts + cell)[0]
        if not previous_end < start <= end:
            raise ListingError("font_invalid_table", "cmap")
        previous_end = end
        delta = struct.unpack_from(">h", subtable, deltas + cell)[0]
        glyphs = _segment_glyphs(subtable, start, end, ranges + cell, ranges + segment_bytes)
        for code, glyph in enumerate(glyphs, start):
            if glyph and code != 0xFFFF:
                glyph_id = (glyph + delta) & 0xFFFF
                if glyph_id >= glyph_count:
                    raise ListingError("font_invalid_table", "cmap")
                mapping[code] = glyph_id
    return mapping
