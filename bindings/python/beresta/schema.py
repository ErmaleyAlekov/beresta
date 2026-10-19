"""
Schema — загруженная XML Schema (неизменяема, потокобезопасна).

    schema = Schema.from_file("order.xsd")
    for problem in schema.validate(document):          # готовое дерево
        print(problem.line, problem.message)
    problems = schema.validate_file("export.xml")      # поток без дерева

Schema — a loaded XML Schema (immutable, thread-safe).
    schema = Schema.from_file("order.xsd")
    for problem in schema.validate(document):          # a ready tree
        print(problem.line, problem.message)
    problems = schema.validate_file("export.xml")      # a stream without a tree
"""

from __future__ import annotations

import os
import weakref
from typing import List, Union

from ._diagnostics import Diagnostic, take_diagnostics
from ._native import LIB, encode, out_handle
from ._schema_streaming import SchemaStreaming
from .errors import check

__all__ = ["Diagnostic", "Schema"]


class Schema(SchemaStreaming):
    """Создаётся только фабричными методами from_string / from_file.
    Потоковая проверка без дерева — validate_bytes / validate_file (SchemaStreaming).
    Created only by the factory methods from_string / from_file.
    Streaming validation without a tree is validate_bytes / validate_file (SchemaStreaming)."""

    def __init__(self, pointer: int) -> None:
        self.handle = pointer
        self._finalizer = weakref.finalize(self, LIB.beresta_schema_free, pointer)

    @classmethod
    def from_string(cls, source: Union[str, bytes]) -> "Schema":
        """Схема из текста XSD (xs:include здесь недоступен — нет каталога).
        A schema from XSD text (xs:include is unavailable here — there is no directory)."""
        data = source
        if isinstance(source, str):
            data = source.encode("utf-8")
        result = out_handle()
        check(LIB.beresta_schema_load(data, len(data), result))
        return cls(result.value)

    @classmethod
    def from_file(cls, path) -> "Schema":
        """Схема из файла; xs:include / xs:import ищутся рядом с ним.
        A schema from a file; xs:include / xs:import are looked up next to it."""
        result = out_handle()
        check(LIB.beresta_schema_load_file(encode(os.fspath(path)), result))
        return cls(result.value)

    def validate(self, document) -> List[Diagnostic]:
        """Все нарушения за один проход; пустой список — документ корректен.
        All violations in one pass; an empty list means the document is valid."""
        diagnostics = out_handle()
        check(LIB.beresta_schema_validate(self.handle, document.handle, diagnostics))
        return take_diagnostics(diagnostics)
