"""Создание документов: разбор текста, байтов и файлов, документ с нуля."""

from __future__ import annotations

import os
from typing import Optional, Union

from ._native import LIB, encode, out_handle
from .document import Document
from .errors import check
from .flags import ParseFlags


def _schema_handle(schema) -> Optional[int]:
    """Дескриптор схемы или NULL, если проверка по схеме не нужна."""
    if schema is None:
        return None
    return schema.handle


def parse(source: Union[str, bytes], flags: ParseFlags = ParseFlags.DEFAULT, schema=None) -> Document:
    """
    Разбор XML. str кодируется в UTF-8; bytes передаются как есть —
    кодировку (UTF-16, windows-1251, ...) библиотека определит сама.
    """
    data = source
    if isinstance(source, str):
        data = source.encode("utf-8")
    result = out_handle()
    check(LIB.beresta_document_parse(data, len(data), int(flags), _schema_handle(schema), result))
    return Document(result.value)


def parse_file(path, flags: ParseFlags = ParseFlags.DEFAULT, schema=None) -> Document:
    """Разбор файла (ParseFlags.FILE_RESOURCES разрешает внешние DTD рядом с ним)."""
    result = out_handle()
    status = LIB.beresta_document_parse_file(encode(os.fspath(path)), int(flags), _schema_handle(schema), result)
    check(status)
    return Document(result.value)


def new_document(root_name: Optional[str] = None) -> Document:
    """Новый документ; root_name=None — без корневого элемента."""
    result = out_handle()
    check(LIB.beresta_document_create(encode(root_name), result))
    return Document(result.value)
