"""Низкоуровневая запись файла PDF: нумерованные объекты, потоки и таблица перекрёстных ссылок.

Паттерн Builder (GoF): документ собирается по частям — объекты добавляются или
резервируются (номер нужен раньше содержимого, например у дерева страниц), а файл
со смещениями объектов (xref) строится одним вызовом write. Вывод детерминирован:
одинаковые объекты дают побайтно одинаковый файл.

Low-level PDF file writing: numbered objects, streams and the cross-reference table.
The Builder pattern (GoF): the document is assembled piece by piece — objects are added or
reserved (a number is needed before the content, for example for the page tree), and the file
with the object offsets (xref) is built by one write call. The output is deterministic:
equal objects give a byte-for-byte equal file.
"""

from __future__ import annotations

import zlib
from pathlib import Path


class PdfWriter:
    """Собиратель объектов PDF 1.4 (версия, которой требует PDF/A-1).
    An assembler of PDF 1.4 objects (the version PDF/A-1 requires)."""

    def __init__(self) -> None:
        # Тела объектов по порядку: номер объекта = индекс + 1; None — зарезервирован
        # The object bodies in order: the object number = index + 1; None means reserved
        self.objects = []

    def add(self, body: bytes = None) -> int:
        """Новый объект (или резерв номера без тела); возвращает его номер.
        A new object (or a number reservation without a body); returns its number."""
        self.objects.append(body)
        return len(self.objects)

    def put(self, number: int, body: bytes) -> None:
        """Тело для ранее зарезервированного объекта.
        The body for a previously reserved object."""
        self.objects[number - 1] = body

    @staticmethod
    def stream(dictionary: str, data: bytes, compress: bool = True) -> bytes:
        """Тело объекта-потока; compress — сжатие Flate (метаданные XMP сжимать нельзя).
        The body of a stream object; compress means Flate compression (XMP metadata must not be compressed)."""
        if compress:
            # Уровень 9 и без отметки времени — результат одинаков при каждом запуске
            # Level 9 and no timestamp — the result is the same on every run
            data = zlib.compress(data, 9)
            dictionary = dictionary + " /Filter /FlateDecode"
        head = "<< " + dictionary + " /Length " + str(len(data)) + " >>\nstream\n"
        return head.encode("latin-1") + data + b"\nendstream"

    def write(self, path: Path, root: int, info: int, file_id: str) -> None:
        """Запись файла: заголовок, объекты, xref, трейлер с /Root, /Info и /ID.
        Writes the file: the header, the objects, xref, the trailer with /Root, /Info and /ID."""
        # Вторая строка с байтами > 127 — признак двоичного файла (требование PDF/A)
        # The second line with bytes > 127 marks a binary file (a PDF/A requirement)
        output = bytearray(b"%PDF-1.4\n%\xe2\xe3\xcf\xd3\n")
        offsets = []
        for number, body in enumerate(self.objects, start=1):
            offsets.append(len(output))
            output += str(number).encode("ascii") + b" 0 obj\n" + body + b"\nendobj\n"
        start = len(output)
        output += ("xref\n0 " + str(len(self.objects) + 1) + "\n0000000000 65535 f \n").encode("ascii")
        for offset in offsets:
            output += ("%010d 00000 n \n" % offset).encode("ascii")
        trailer = "trailer\n<< /Size %d /Root %d 0 R /Info %d 0 R /ID [<%s> <%s>] >>\nstartxref\n%d\n%%%%EOF\n"
        output += (trailer % (len(self.objects) + 1, root, info, file_id, file_id, start)).encode("ascii")
        path.write_bytes(bytes(output))
