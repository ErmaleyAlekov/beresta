"""
Ошибки библиотеки в виде исключений Python.

Си-интерфейс сообщает об ошибке кодом состояния и «последней ошибкой
потока». Функции этого модуля превращают их в BerestaError — так код,
использующий обёртку, обрабатывает сбои привычным try/except.

Library errors as Python exceptions.
The C interface reports an error with a status code and the "last error of
the thread". The functions of this module turn them into BerestaError — so
code using the wrapper handles failures with the usual try/except.
"""

from __future__ import annotations

import ctypes
from typing import Optional

from ._native import LIB


class BerestaError(Exception):
    """Ошибка «Берёсты»: code — число beresta_status, name — его имя.
    A Beresta error: code is the beresta_status number, name is its name."""

    def __init__(self, code: int, message: str) -> None:
        self.code = code
        self.name = LIB.beresta_status_name(code).decode("utf-8")
        self.message = message
        super().__init__("[" + self.name + "] " + message)


def raise_last_error() -> None:
    """Бросить BerestaError по последней ошибке текущего потока.
    Raises BerestaError for the last error of the current thread."""
    code = LIB.beresta_last_error_code()
    message = LIB.beresta_last_error_message().decode("utf-8", errors="replace")
    raise BerestaError(code, message)


def check(status: int) -> None:
    """Проверка int-статуса: всё, кроме BERESTA_OK (0), — исключение.
    Checks an int status: anything except BERESTA_OK (0) is an exception."""
    if status != 0:
        raise_last_error()


def check_handle(pointer: Optional[int]) -> Optional[int]:
    """
    Проверка дескриптора-результата. NULL без ошибки — законный ответ
    «объекта нет» (например, у последнего узла нет соседа) -> None.

    Checks a handle result. NULL without an error is a legitimate answer
    "there is no object" (for example, the last node has no sibling) -> None.
    """
    if pointer is None and LIB.beresta_last_error_code() != 0:
        raise_last_error()
    return pointer


def take_text(pointer: Optional[int]) -> Optional[str]:
    """
    Строка char*, принадлежащая вызывающему: копируется в str и сразу
    освобождается через beresta_string_free — утечки памяти нет.

    A char* string owned by the caller: copied into str and freed at once
    through beresta_string_free — no memory leaks.
    """
    if pointer is None:
        check_handle(pointer)
        return None
    try:
        return ctypes.string_at(pointer).decode("utf-8")
    finally:
        LIB.beresta_string_free(pointer)
