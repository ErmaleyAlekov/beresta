"""
Schema — загруженная XML Schema (неизменяема, потокобезопасна).

    schema = Schema.from_file("order.xsd")
    for problem in schema.validate(document):
        print(problem.line, problem.message)
"""

from __future__ import annotations

import os
import weakref
from dataclasses import dataclass
from typing import List, Union

from ._native import LIB, encode, out_handle
from .errors import check, take_text


@dataclass(frozen=True)
class Diagnostic:
    """Одно нарушение: текст с путём к элементу и место в исходнике (0 — неизвестно)."""

    message: str
    line: int
    column: int


class Schema:
    """Создаётся только фабричными методами from_string / from_file."""

    def __init__(self, pointer: int) -> None:
        self.handle = pointer
        self._finalizer = weakref.finalize(self, LIB.beresta_schema_free, pointer)

    @classmethod
    def from_string(cls, source: Union[str, bytes]) -> "Schema":
        """Схема из текста XSD (xs:include здесь недоступен — нет каталога)."""
        data = source
        if isinstance(source, str):
            data = source.encode("utf-8")
        result = out_handle()
        check(LIB.beresta_schema_load(data, len(data), result))
        return cls(result.value)

    @classmethod
    def from_file(cls, path) -> "Schema":
        """Схема из файла; xs:include / xs:import ищутся рядом с ним."""
        result = out_handle()
        check(LIB.beresta_schema_load_file(encode(os.fspath(path)), result))
        return cls(result.value)

    def validate(self, document) -> List[Diagnostic]:
        """Все нарушения за один проход; пустой список — документ корректен."""
        diagnostics = out_handle()
        check(LIB.beresta_schema_validate(self.handle, document.handle, diagnostics))
        try:
            return [_diagnostic_at(diagnostics, index)
                    for index in range(LIB.beresta_diagnostics_count(diagnostics))]
        finally:
            LIB.beresta_diagnostics_free(diagnostics)


def _diagnostic_at(diagnostics, index: int) -> Diagnostic:
    """Чтение одного нарушения из нативного списка."""
    message = take_text(LIB.beresta_diagnostics_message(diagnostics, index))
    line = LIB.beresta_diagnostics_line(diagnostics, index)
    column = LIB.beresta_diagnostics_column(diagnostics, index)
    return Diagnostic(message, line, column)
