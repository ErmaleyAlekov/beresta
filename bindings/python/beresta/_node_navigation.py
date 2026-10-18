"""Примесь: навигация по дереву (родитель, соседи, первый и последний ребёнок).
Mixin: tree navigation (parent, siblings, first and last child)."""

from __future__ import annotations

from ._native import LIB
from ._wrapping import wrap_node


class NodeNavigationMixin:
    """Соседи и родитель находятся за O(1): ядро хранит индекс узла у родителя.
    Siblings and the parent are found in O(1): the core stores the node's index in its parent."""

    @property
    def parent(self):
        return wrap_node(LIB.beresta_node_parent(self.handle), self.document)

    @property
    def first_child(self):
        return wrap_node(LIB.beresta_node_first_child(self.handle), self.document)

    @property
    def last_child(self):
        return wrap_node(LIB.beresta_node_last_child(self.handle), self.document)

    @property
    def next_sibling(self):
        return wrap_node(LIB.beresta_node_next_sibling(self.handle), self.document)

    @property
    def previous_sibling(self):
        return wrap_node(LIB.beresta_node_previous_sibling(self.handle), self.document)
