"""Регрессии инструментов листинга и Unicode (Т-39).
Listing and Unicode tools regressions (T-39)."""

import io
import struct
import sys
import tempfile
import unittest
from datetime import datetime
from pathlib import Path
from unittest.mock import patch
from xml.etree import ElementTree

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "legal"))
sys.path.insert(0, str(ROOT))

import make_listing
from listing.errors import ListingError
from listing.pdf_metadata import pdf_text, write_metadata, xmp_packet
from listing.pdf_pages import page_geometry
from listing.pdf_writer import PdfWriter
from listing.ttf_reader import load_font
from tools.unicode.generate_unicode_tables import UnicodeDataError, load_categories


def _font_bytes(hmetrics=1, fs_type=0, extra=None):
    """Минимальные таблицы, которые читает инструмент листинга.
    The minimum tables read by the listing tool."""
    head = bytearray(44)
    hhea = bytearray(36)
    os2 = bytearray(10)
    struct.pack_into(">H", head, 18, 1000)
    struct.pack_into(">H", hhea, 34, hmetrics)
    struct.pack_into(">H", os2, 8, fs_type)
    cmap = struct.pack(">2H2HI", 0, 1, 3, 1, 12)
    cmap += struct.pack(">16H", 4, 32, 0, 4, 4, 1, 0, 65, 0xFFFF, 0, 65, 0xFFFF, 0xFFC0, 1, 0, 0)
    tables = {b"head": head, b"hhea": hhea, b"hmtx": b"\x02\x00\0\0\0\0",
              b"maxp": struct.pack(">IH", 0x10000, 2), b"cmap": cmap, b"OS/2": os2, b"post": bytes(8)}
    tables.update(extra or {})
    header = struct.pack(">IH6x", 0x10000, len(tables))
    payload = bytearray()
    for tag, value in tables.items():
        header += struct.pack(">4sIII", tag, 0, 12 + 16 * len(tables) + len(payload), len(value))
        payload.extend(value)
    return header + payload


class TrueTypeTests(unittest.TestCase):
    """Проверка повреждённых и корректных шрифтов через загрузчик.
    Test malformed and valid fonts through the loader."""

    def _load(self, data):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "test.ttf"
            path.write_bytes(data)
            return load_font(path)

    def test_valid_font_keeps_width_and_mapping(self):
        font = self._load(_font_bytes())
        self.assertEqual(font["widths"], [512, 512])
        self.assertEqual(font["cmap"], {65: 1})

    def test_rejects_truncated_directory_and_tables(self):
        font = _font_bytes()
        for data in (font[:10], font[:20], font[:-1],
                     _font_bytes(extra={b"head": bytes(18)}),
                     _font_bytes(extra={b"cmap": bytes(2)})):
            with self.subTest(length=len(data)):
                with self.assertRaises(ListingError):
                    self._load(data)

    def test_rejects_zero_horizontal_metrics(self):
        with self.assertRaises(ListingError):
            self._load(_font_bytes(hmetrics=0))

    def test_column_must_fit_the_page(self):
        for width in (0, 65535):
            with self.subTest(width=width):
                font = self._load(_font_bytes(extra={b"hmtx": struct.pack(">H4x", width)}))
                with self.assertRaises(ListingError):
                    page_geometry(font, 10, 12, 50)

    def test_rejects_invalid_name_record_count_and_string_bounds(self):
        bad_count = struct.pack(">3H", 0, 4, 6)
        bad_offset = struct.pack(">9H", 0, 1, 18, 3, 1, 0, 6, 2, 4) + b"\0A"
        for name in (b"", bad_count, bad_offset):
            with self.subTest(name=name):
                with self.assertRaises(ListingError):
                    self._load(_font_bytes(extra={b"name": name}))

    def test_rejects_bitmap_only_embedding(self):
        with self.assertRaises(ListingError):
            self._load(_font_bytes(fs_type=0x0200))


class ListingMetadataTests(unittest.TestCase):
    """Проверка метаданных PDF и перенаправления вывода.
    Test PDF metadata and redirected output."""

    def test_control_characters_share_a_valid_title(self):
        title = "A\x00\x07\x0c\x7f\r <&> Ё"
        expected = "A\ufffd\ufffd\ufffd\ufffd\r <&> Ё"
        moment = datetime(2026, 11, 21)
        writer = PdfWriter()
        numbers = write_metadata(writer, title, moment, None)
        info = writer.objects[numbers["info"] - 1]
        self.assertIn(pdf_text(expected).encode("ascii"), info)
        packet = writer.objects[numbers["metadata"] - 1].split(b"\nstream\n", 1)[1].rsplit(b"\nendstream", 1)[0]
        namespaces = {"rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#"}
        for xmp in (packet, xmp_packet(title, moment, True)):
            self.assertEqual(ElementTree.fromstring(xmp).find(".//rdf:li", namespaces).text, expected)

    def test_main_accepts_output_streams_without_reconfigure(self):
        with tempfile.TemporaryDirectory() as temp:
            argv = ["make_listing.py", "--font", str(Path(temp) / "absent.ttf"), "--lang", "en"]
            output, errors = io.StringIO(), io.StringIO()
            with patch.object(sys, "argv", argv), patch.object(sys, "stdout", output), \
                    patch.object(sys, "stderr", errors):
                self.assertEqual(make_listing.main(), 1)
            self.assertIn("--font", errors.getvalue())


class UnicodeRangeTests(unittest.TestCase):
    """Проверка пар границ диапазонов UnicodeData.
    Test UnicodeData range boundary pairs."""

    def _load(self, data):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "UnicodeData-3.1.0.txt"
            path.write_text(data, encoding="utf-8")
            return load_categories(path)

    def test_pairs_can_merge_with_adjacent_records(self):
        source = "0040;BEFORE;Lo\n0041;<Test, First>;Lo\n0043;<Test, Last>;Lo\n0044;AFTER;Lo\n"
        self.assertEqual(self._load(source), [[0x40, 0x44, "Lo"]])

    def test_rejects_unpaired_and_mismatched_boundaries(self):
        first, last = "0041;<Test, First>;Lo\n", "0043;<Test, Last>;Lo\n"
        for source, line in ((last, 1), (first, 1), (first + last + last, 3),
                             (first + last.replace("Test", "Other"), 2),
                             (first + last.replace("Lo", "Lu"), 2),
                             (first + last.replace("0043", "0040"), 2)):
            with self.subTest(source=source):
                with self.assertRaisesRegex(UnicodeDataError, r"UnicodeData-3\.1\.0\.txt:%d:" % line):
                    self._load(source)
