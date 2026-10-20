"""Собственная ошибка генератора листинга: несёт готовое сообщение из каталога.
Точка входа ловит только её и печатает текст без трассировки стека.

The listing generator's own error: it carries a ready message from the catalog.
The entry point catches only it and prints the text without a stack trace.
"""

from __future__ import annotations

from .messages import text


class ListingError(Exception):
    """Ошибка подготовки листинга: шрифт, исходники, версия.
    A listing preparation error: the font, the sources, the version."""

    def __init__(self, identifier: str, *arguments: object) -> None:
        # Сообщение берётся из каталога по идентификатору
        # The message is taken from the catalog by its identifier
        super().__init__(text(identifier, *arguments))
