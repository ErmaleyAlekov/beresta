"""
Document — документ XML, владелец всего дерева узлов.

Нативная память освобождается детерминированно — через close() или блок
with — либо, если про это забыли, сборщиком мусора: weakref.finalize
гарантирует ровно один вызов beresta_document_free в любом случае.
"""

from __future__ import annotations

import weakref

from ._document_content import DocumentContentMixin
from ._native import LIB
from .node import Node


class Document(Node, DocumentContentMixin):
    """Документ: сам является узлом (к нему можно добавлять детей)."""

    def __init__(self, pointer: int) -> None:
        super().__init__(pointer, self)
        self._finalizer = weakref.finalize(self, LIB.beresta_document_free, pointer)

    @property
    def closed(self) -> bool:
        """True после close(): память документа уже освобождена."""
        return not self._finalizer.alive

    def close(self) -> None:
        """Освобождение нативной памяти (повторный вызов безопасен)."""
        self._finalizer()

    def __enter__(self) -> "Document":
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self.close()
