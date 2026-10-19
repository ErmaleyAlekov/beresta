"""
Примесь Schema: потоковая проверка без построения дерева (SAX + XSD
внутри ядра). Память — O(глубины документа) плюс значения ID и таблицы
ключей; файл отображается в память, а не копируется. Так проверяются
выгрузки в гигабайты, которые в дерево не помещаются, и это быстрее,
чем parse + validate: дерево не строится вовсе.

    for problem in schema.validate_file("export.xml"):
        print(problem.line, problem.message)

Нарушения схемы возвращаются списком; синтаксическая ошибка XML —
исключение BerestaError (PARSE_ERROR).

A Schema mixin: streaming validation without building a tree (SAX + XSD
inside the core). The memory is O(document depth) plus ID values and key
tables; the file is mapped into memory, not copied. This is how exports of
gigabytes that do not fit into a tree are validated, and it is faster than
parse + validate: no tree is built at all.
    for problem in schema.validate_file("export.xml"):
        print(problem.line, problem.message)
Schema violations are returned as a list; an XML syntax error is a
BerestaError (PARSE_ERROR).
"""

from __future__ import annotations

import os
from typing import List, Union

from ._diagnostics import Diagnostic, take_diagnostics
from ._native import LIB, encode, out_handle
from .errors import check
from .flags import ParseFlags


class SchemaStreaming:
    """Методы потоковой проверки; дескриптор схемы — self.handle.
    Streaming validation methods; the schema handle is self.handle."""

    def validate_bytes(self, source: Union[str, bytes],
                       flags: ParseFlags = ParseFlags.DEFAULT) -> List[Diagnostic]:
        """Проверка XML из памяти: str кодируется в UTF-8, bytes передаются как есть.
        Validates XML from memory: str is encoded to UTF-8, bytes are passed as is."""
        data = source
        if isinstance(source, str):
            data = source.encode("utf-8")
        diagnostics = out_handle()
        check(LIB.beresta_schema_validate_bytes(data, len(data), int(flags), self.handle, diagnostics))
        return take_diagnostics(diagnostics)

    def validate_file(self, path, flags: ParseFlags = ParseFlags.DEFAULT) -> List[Diagnostic]:
        """Проверка файла (ParseFlags.FILE_RESOURCES разрешает внешние DTD рядом с ним).
        Validates a file (ParseFlags.FILE_RESOURCES allows external DTDs next to it)."""
        diagnostics = out_handle()
        status = LIB.beresta_schema_validate_file(encode(os.fspath(path)), int(flags), self.handle, diagnostics)
        check(status)
        return take_diagnostics(diagnostics)
