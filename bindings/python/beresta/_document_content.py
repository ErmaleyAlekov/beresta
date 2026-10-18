"""Примесь документа: корень, поиск по ID, копия, запись в файл.
Document mixin: the root, lookup by ID, a copy, writing to a file."""

from __future__ import annotations

import os
from typing import Optional

from ._native import LIB, encode, out_handle
from ._wrapping import wrap_node
from .errors import check
from .flags import WriteFlags


class DocumentContentMixin:
    """Операции уровня документа. / Document-level operations."""

    @property
    def raw_handle(self) -> int:
        """Дескриптор без проверки закрытия (нужен фабрике обёрток).
        The handle without the closed check (needed by the wrapper factory)."""
        return self._pointer

    @property
    def root(self):
        """Корневой элемент или None. / The root element or None."""
        return wrap_node(LIB.beresta_document_root(self.handle), self)

    def element_by_id(self, identifier: str):
        """Элемент с атрибутом типа ID (по DTD) или xml:id.
        An element with an ID-typed attribute (by DTD) or xml:id."""
        return wrap_node(LIB.beresta_document_element_by_id(self.handle, encode(identifier)), self)

    def clone(self):
        """Независимая глубокая копия документа. / An independent deep copy of the document."""
        from .document import Document

        result = out_handle()
        check(LIB.beresta_document_clone(self.handle, result))
        return Document(result.value)

    def save(self, path, flags: WriteFlags = WriteFlags.DEFAULT, encoding: Optional[str] = None) -> None:
        """Запись в файл; encoding=None — UTF-8, иначе, например, "windows-1251".
        Writing to a file; encoding=None means UTF-8, otherwise, for example, "windows-1251"."""
        check(LIB.beresta_document_save(self.handle, encode(os.fspath(path)), int(flags), encode(encoding)))
