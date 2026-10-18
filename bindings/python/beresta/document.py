"""
Document — документ XML, владелец всего дерева узлов.

Нативная память освобождается детерминированно — через close() или блок
with — либо, если про это забыли, сборщиком мусора: weakref.finalize
гарантирует ровно один вызов beresta_document_free в любом случае.

Document — an XML document, the owner of the whole node tree.
The native memory is freed deterministically — through close() or a with
block — or, if that was forgotten, by the garbage collector: weakref.finalize
guarantees exactly one call of beresta_document_free in any case.
"""

from __future__ import annotations

import weakref

from ._document_content import DocumentContentMixin
from ._native import LIB
from .node import Node


class Document(Node, DocumentContentMixin):
    """Документ: сам является узлом (к нему можно добавлять детей).
    A document: it is itself a node (children can be added to it)."""

    def __init__(self, pointer: int) -> None:
        super().__init__(pointer, self)
        self._finalizer = weakref.finalize(self, LIB.beresta_document_free, pointer)
        # Единственные обёртки узлов документа: адрес -> обёртка (см. _wrapping.py)
        # The only wrappers of the document nodes: address -> wrapper (see _wrapping.py)
        self._wrappers = weakref.WeakValueDictionary()

    @property
    def closed(self) -> bool:
        """True после close(): память документа уже освобождена.
        True after close(): the document memory is already freed."""
        return not self._finalizer.alive

    def close(self) -> None:
        """Освобождение нативной памяти (повторный вызов безопасен).
        Releases the native memory (calling it again is safe)."""
        self._finalizer()

    def __enter__(self) -> "Document":
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self.close()
