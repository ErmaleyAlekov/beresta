"""
Язык сообщений об ошибках. По умолчанию — язык, выбранный при сборке
нативной библиотеки (ERRORS_LANG); сменить можно в любой момент:

    beresta.set_language("en")

Language of error messages. The default is the language chosen when the
native library was built (ERRORS_LANG); it can be changed at any time:

    beresta.set_language("en")
"""

from __future__ import annotations

from typing import List

from ._native import LIB, encode
from .errors import check

# Номера сообщений обёрток (beresta_message в beresta_c.h)
# Binding message numbers (beresta_message in beresta_c.h)
DOCUMENT_CLOSED = 1
XPATH_CLOSED = 2
SCHEMA_CLOSED = 3


def set_language(code: str) -> None:
    """Смена языка для всех потоков; неизвестный код — BerestaError.
    Changes the language for all threads; an unknown code raises BerestaError."""
    check(LIB.beresta_set_language(encode(code)))


def language() -> str:
    """Код текущего языка, например "ru". / The current language code, for example "ru"."""
    return LIB.beresta_language().decode("ascii")


def languages() -> List[str]:
    """Все языки библиотеки. / All languages of the library."""
    return LIB.beresta_languages().decode("ascii").split(",")


def message_text(message: int, *arguments: str) -> str:
    """Сообщение обёртки на текущем языке с подставленными {0}, {1}...
    A binding message in the current language with {0}, {1}... substituted."""
    text = LIB.beresta_message_text(message).decode("utf-8")
    for index, argument in enumerate(arguments):
        text = text.replace("{" + str(index) + "}", argument)
    return text
