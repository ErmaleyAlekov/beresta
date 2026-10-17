"""Примесь документа: корень, поиск по ID, копия, запись в файл."""

from __future__ import annotations

import os
from typing import Optional

from ._native import LIB, encode, out_handle
from ._wrapping import wrap_node
from .errors import check
from .flags import WriteFlags


class DocumentContentMixin:
    """Операции уровня документа."""

    @property
    def raw_handle(self) -> int:
        """Дескриптор без проверки закрытия (нужен фабрике обёрток)."""
        return self._pointer

    @property
    def root(self):
        """Корневой элемент или None."""
        return wrap_node(LIB.beresta_document_root(self.handle), self)

    def element_by_id(self, identifier: str):
        """Элемент с атрибутом типа ID (по DTD) или xml:id."""
        return wrap_node(LIB.beresta_document_element_by_id(self.handle, encode(identifier)), self)

    def clone(self):
        """Независимая глубокая копия документа."""
        from .document import Document

        result = out_handle()
        check(LIB.beresta_document_clone(self.handle, result))
        return Document(result.value)

    def save(self, path, flags: WriteFlags = WriteFlags.DEFAULT, encoding: Optional[str] = None) -> None:
        """Запись в файл; encoding=None — UTF-8, иначе, например, "windows-1251"."""
        check(LIB.beresta_document_save(self.handle, encode(os.fspath(path)), int(flags), encode(encoding)))
