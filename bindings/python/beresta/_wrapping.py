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
поиске сверяется тип узла, запомненный в обёртке (текст, комментарий,
элемент...). Тот же тип обёртка отдаёт из .type без нового вызова ctypes.

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
outlive it (the address goes to a new node), so the lookup compares the node type
remembered in the wrapper (a text, a comment, an element...). The wrapper returns
the same type from .type without a new ctypes call.
"""

from __future__ import annotations

from typing import Optional

from ._native import LIB
from .errors import check_handle
from .flags import NodeType

# Код типа «элемент» как обычное int: сравнение без перечисления на каждый узел
# The "element" type code as a plain int: a comparison without the enum for every node
_ELEMENT = int(NodeType.ELEMENT)

# Классы обёрток (Element, Node); заполняется один раз в _wrapper_classes
# The wrapper classes (Element, Node); filled once in _wrapper_classes
_CLASSES: list = []


def _wrapper_classes() -> list:
    """Импорт внутри функции разрывает циклическую зависимость модулей, а
    список-кеш выполняет его один раз: раньше import стоял на каждый узел
    и отнимал около десятой части времени обхода.
    The import inside the function breaks the circular dependency of the modules,
    and the cache list runs it once: the import used to run for every node and
    took about a tenth of the walk time."""
    if not _CLASSES:
        from .element import Element
        from .node import Node
        _CLASSES.extend((Element, Node))
    return _CLASSES


def wrap_node(pointer: Optional[int], document):
    """Обёртка для дескриптора узла документа document (None -> None).
    A wrapper for a node handle of the document document (None -> None)."""
    check_handle(pointer)
    if pointer is None:
        return None
    if pointer == document.raw_handle:
        return document
    kind = LIB.beresta_node_type(pointer)
    known = document._wrappers.get(pointer)
    if known is not None and known._kind == kind:
        return known
    element_class, node_class = _wrapper_classes()
    if kind == _ELEMENT:
        node = element_class(pointer, document, kind)
    else:
        node = node_class(pointer, document, kind)
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
