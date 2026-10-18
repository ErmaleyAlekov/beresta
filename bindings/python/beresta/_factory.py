"""Создание документов: разбор текста, байтов и файлов, документ с нуля.
Creating documents: parsing text, bytes and files, a document from scratch."""

from __future__ import annotations

import os
from typing import Optional, Union

from ._native import LIB, encode, out_handle
from .document import Document
from .errors import check
from .flags import ParseFlags


def _schema_handle(schema) -> Optional[int]:
    """Дескриптор схемы или NULL, если проверка по схеме не нужна.
    The schema handle or NULL if no schema validation is needed."""
    if schema is None:
        return None
    return schema.handle


def parse(source: Union[str, bytes], flags: ParseFlags = ParseFlags.DEFAULT, schema=None) -> Document:
    """
    Разбор XML. str кодируется в UTF-8; bytes передаются как есть —
    кодировку (UTF-16, windows-1251, ...) библиотека определит сама.

    Parses XML. str is encoded to UTF-8; bytes are passed as is —
    the library detects the encoding (UTF-16, windows-1251, ...) itself.
    """
    data = source
    if isinstance(source, str):
        data = source.encode("utf-8")
    result = out_handle()
    check(LIB.beresta_document_parse(data, len(data), int(flags), _schema_handle(schema), result))
    return Document(result.value)


def parse_file(path, flags: ParseFlags = ParseFlags.DEFAULT, schema=None) -> Document:
    """Разбор файла (ParseFlags.FILE_RESOURCES разрешает внешние DTD рядом с ним).
    Parses a file (ParseFlags.FILE_RESOURCES allows external DTDs next to it)."""
    result = out_handle()
    status = LIB.beresta_document_parse_file(encode(os.fspath(path)), int(flags), _schema_handle(schema), result)
    check(status)
    return Document(result.value)


def new_document(root_name: Optional[str] = None) -> Document:
    """Новый документ; root_name=None — без корневого элемента.
    A new document; root_name=None means without a root element."""
    result = out_handle()
    check(LIB.beresta_document_create(encode(root_name), result))
    return Document(result.value)
