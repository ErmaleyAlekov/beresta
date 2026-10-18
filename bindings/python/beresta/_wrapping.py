"""
Фабрика объектов-обёрток для дескрипторов узлов (паттерн Factory Method).

Си-интерфейс отдаёт «голый» указатель; по его типу выбирается класс
Python: Element для элементов, сам документ для дескриптора документа,
Node для остальных узлов. Импорт классов внутри функции разрывает
циклическую зависимость модулей node <-> element <-> document.

У одного узла — одна обёртка: документ хранит словарь «адрес -> обёртка»
со слабыми ссылками (паттерн Identity Map). Он нужен для переноса узла
в другой документ (move_to): у ВСЕХ уже выданных обёрток узла и его
потомков должен смениться документ-владелец. Иначе обёртка, полученная
до переноса, проверяла бы «жив ли документ» по старому документу и после
закрытия нового обратилась бы к освобождённой памяти. Запись об удалённом
узле может пережить его (адрес достанется новому узлу), поэтому при
поиске сверяется вид обёртки: элемент или нет.

The factory of wrapper objects for node handles (the Factory Method pattern).
The C interface returns a "bare" pointer; the Python class is chosen by its
type: Element for elements, the document itself for the document handle,
Node for other nodes. Importing the classes inside the function breaks
the circular dependency of the modules node <-> element <-> document.

One node has one wrapper: the document keeps an "address -> wrapper" dictionary
with weak references (the Identity Map pattern). It is needed to move a node into
another document (move_to): ALL wrappers already handed out for the node and its
descendants must change their owner document. Otherwise a wrapper obtained before
the move would check "is the document alive" against the old document and, after
the new one is closed, would access freed memory. The entry of a removed node may
outlive it (the address goes to a new node), so the lookup compares the wrapper
kind: an element or not.
"""

from __future__ import annotations

from typing import Optional

from ._native import LIB
from .errors import check_handle
from .flags import NodeType


def wrap_node(pointer: Optional[int], document):
    """Обёртка для дескриптора узла документа document (None -> None).
    A wrapper for a node handle of the document document (None -> None)."""
    from .element import Element
    from .node import Node

    check_handle(pointer)
    if pointer is None:
        return None
    if pointer == document.raw_handle:
        return document
    element = LIB.beresta_node_type(pointer) == NodeType.ELEMENT
    known = document._wrappers.get(pointer)
    if known is not None and isinstance(known, Element) == element:
        return known
    if element:
        node = Element(pointer, document)
    else:
        node = Node(pointer, document)
    document._wrappers[pointer] = node
    return node


def transfer_wrappers(moved, source, target) -> None:
    """
    Передача обёрток перенесённого поддерева документу target (и самой
    обёртки moved). Узлы уже живут в target, поэтому обход нативного
    поддерева безопасен. Обход итеративный (явный стек): глубина дерева не
    ограничена стеком вызовов. Если у source нет других обёрток, кроме
    корня поддерева, обходить потомков незачем.

    Hands the wrappers of the moved subtree over to the target document (and the
    moved wrapper itself). The nodes already live in target, so walking the native
    subtree is safe. The walk is iterative (an explicit stack): the tree depth is
    not limited by the call stack. If source has no wrappers other than the subtree
    root, there is no need to walk the descendants.
    """
    deep = len(source._wrappers) > 1
    moved._document = target
    pending = [moved._pointer]
    while pending:
        current = pending.pop()
        node = source._wrappers.pop(current, None)
        if node is not None:
            node._document = target
            target._wrappers[current] = node
        child = None
        if deep:
            child = LIB.beresta_node_first_child(current)
        while child is not None:
            pending.append(child)
            child = LIB.beresta_node_next_sibling(child)
