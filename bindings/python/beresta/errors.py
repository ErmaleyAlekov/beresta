"""
Ошибки библиотеки в виде исключений Python.

Си-интерфейс сообщает об ошибке кодом состояния и «последней ошибкой
потока». Функции этого модуля превращают их в BerestaError — так код,
использующий обёртку, обрабатывает сбои привычным try/except.
"""

from __future__ import annotations

import ctypes
from typing import Optional

from ._native import LIB


class BerestaError(Exception):
    """Ошибка «Берёсты»: code — число beresta_status, name — его имя."""

    def __init__(self, code: int, message: str) -> None:
        self.code = code
        self.name = LIB.beresta_status_name(code).decode("utf-8")
        self.message = message
        super().__init__("[" + self.name + "] " + message)


def raise_last_error() -> None:
    """Бросить BerestaError по последней ошибке текущего потока."""
    code = LIB.beresta_last_error_code()
    message = LIB.beresta_last_error_message().decode("utf-8", errors="replace")
    raise BerestaError(code, message)


def check(status: int) -> None:
    """Проверка int-статуса: всё, кроме BERESTA_OK (0), — исключение."""
    if status != 0:
        raise_last_error()


def check_handle(pointer: Optional[int]) -> Optional[int]:
    """
    Проверка дескриптора-результата. NULL без ошибки — законный ответ
    «объекта нет» (например, у последнего узла нет соседа) -> None.
    """
    if pointer is None and LIB.beresta_last_error_code() != 0:
        raise_last_error()
    return pointer


def take_text(pointer: Optional[int]) -> Optional[str]:
    """
    Строка char*, принадлежащая вызывающему: копируется в str и сразу
    освобождается через beresta_string_free — утечки памяти нет.
    """
    if pointer is None:
        check_handle(pointer)
        return None
    try:
        return ctypes.string_at(pointer).decode("utf-8")
    finally:
        LIB.beresta_string_free(pointer)
