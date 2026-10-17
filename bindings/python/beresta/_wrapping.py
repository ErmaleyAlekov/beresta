"""
Фабрика объектов-обёрток для дескрипторов узлов (паттерн Factory Method).

Си-интерфейс отдаёт «голый» указатель; по его типу выбирается класс
Python: Element для элементов, сам документ для дескриптора документа,
Node для остальных узлов. Импорт классов внутри функции разрывает
циклическую зависимость модулей node <-> element <-> document.
"""

from __future__ import annotations

from typing import Optional

from ._native import LIB
from .errors import check_handle
from .flags import NodeType


def wrap_node(pointer: Optional[int], document):
    """Обёртка для дескриптора узла документа document (None -> None)."""
    from .element import Element
    from .node import Node

    check_handle(pointer)
    if pointer is None:
        return None
    if pointer == document.raw_handle:
        return document
    if LIB.beresta_node_type(pointer) == NodeType.ELEMENT:
        return Element(pointer, document)
    return Node(pointer, document)
