"""Примесь: доступ к детям как к последовательности Python. / Mixin: access to children as a Python sequence."""

from __future__ import annotations

from typing import Iterator

from ._native import LIB
from ._wrapping import wrap_node
from .errors import check


class NodeChildrenMixin:
    """len(node), node[i], for child in node: ..."""

    @property
    def child_count(self) -> int:
        count = LIB.beresta_node_child_count(self.handle)
        if count < 0:
            check(LIB.beresta_last_error_code())
        return count

    @property
    def children(self) -> list:
        """Список детей (снимок: последующие изменения дерева в нём не видны).
        The list of children (a snapshot: later tree changes are not visible in it)."""
        return list(iter(self))

    def __len__(self) -> int:
        return self.child_count

    def __getitem__(self, index: int):
        """Ребёнок по индексу; отрицательный индекс — с конца, как у list.
        A child by index; a negative index counts from the end, as in list."""
        if index < 0:
            index += self.child_count
        return wrap_node(LIB.beresta_node_child(self.handle, index), self.document)

    def __iter__(self) -> Iterator:
        """Обход детей через соседей — O(1) на шаг, без лишних списков.
        Iterates children through siblings — O(1) per step, without extra lists."""
        child = self.first_child
        while child is not None:
            yield child
            child = child.next_sibling
