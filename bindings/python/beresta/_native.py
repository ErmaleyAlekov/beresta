"""
Единственный экземпляр загруженной библиотеки (паттерн Singleton на уровне
модуля: Python импортирует модуль ровно один раз, поэтому библиотека
загружается и связывается с сигнатурами тоже один раз).

The single instance of the loaded library (the Singleton pattern at module
level: Python imports a module exactly once, so the library is also
loaded and bound to the signatures once).
"""

from __future__ import annotations

from ctypes import c_void_p
from typing import Optional

from ._loader import load_library
from ._signatures import bind_signatures


def _initialize():
    """Загрузка библиотеки и назначение сигнатур всем функциям.
    Loads the library and assigns signatures to all functions."""
    library = load_library()
    bind_signatures(library)
    return library


LIB = _initialize()


def encode(text: Optional[str]) -> Optional[bytes]:
    """Строка Python -> байты UTF-8 для параметра const char* (None -> NULL).
    A Python string -> UTF-8 bytes for a const char* parameter (None -> NULL)."""
    if text is None:
        return None
    return str(text).encode("utf-8")


def out_handle() -> c_void_p:
    """Пустая ячейка для выходного параметра вида beresta_document* out.
    An empty cell for an output parameter like beresta_document* out."""
    return c_void_p()
