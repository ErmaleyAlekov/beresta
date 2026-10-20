"""Собственная ошибка сборки документов: текст берётся из каталога сообщений по идентификатору.
The documents build's own error: the text is taken from the message catalog by its identifier."""

from __future__ import annotations

from .messages import text


class DocsError(Exception):
    """Ошибка сборки документов реестра (нет браузера, сбой печати, нет файла).
    A registry documents build error (no browser, a printing failure, a missing file)."""

    def __init__(self, identifier: str, *arguments: object) -> None:
        # Сообщение на текущем языке из каталога / The message in the current language from the catalog
        super().__init__(text(identifier, *arguments))
