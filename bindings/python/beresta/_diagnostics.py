"""
Нарушения схемы и чтение их нативного списка. Общее для проверки
готового дерева (Schema.validate) и потоковой проверки байтов и файлов
(Schema.validate_bytes / validate_file) — одна функция вместо трёх копий.

Schema violations and reading their native list. Common for the validation of
a ready tree (Schema.validate) and the streaming validation of bytes and files
(Schema.validate_bytes / validate_file) — one function instead of three copies.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import List

from ._native import LIB
from .errors import take_text


@dataclass(frozen=True)
class Diagnostic:
    """Одно нарушение: текст с путём к элементу и место в исходнике (0 — неизвестно).
    One violation: the text with the element path and the place in the source (0 — unknown)."""

    message: str
    line: int
    column: int


def take_diagnostics(diagnostics) -> List[Diagnostic]:
    """Список нарушений из нативного дескриптора; дескриптор освобождается в finally.
    The list of violations from a native handle; the handle is freed in finally."""
    try:
        return [_diagnostic_at(diagnostics, index)
                for index in range(LIB.beresta_diagnostics_count(diagnostics))]
    finally:
        LIB.beresta_diagnostics_free(diagnostics)


def _diagnostic_at(diagnostics, index: int) -> Diagnostic:
    """Чтение одного нарушения из нативного списка. / Reads one violation from the native list."""
    message = take_text(LIB.beresta_diagnostics_message(diagnostics, index))
    line = LIB.beresta_diagnostics_line(diagnostics, index)
    column = LIB.beresta_diagnostics_column(diagnostics, index)
    return Diagnostic(message, line, column)
