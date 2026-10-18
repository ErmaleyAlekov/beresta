"""Примесь: добавление новых узлов в конец детей. / Mixin: appending new nodes to the end of the children."""

from __future__ import annotations

from typing import Optional

from ._native import LIB, encode
from ._wrapping import wrap_node


class NodeEditingMixin:
    """Каждый метод возвращает созданный узел — удобно строить дерево цепочкой.
    Each method returns the created node — convenient for building a tree in a chain."""

    def append_element(self, name: str):
        """Новый элемент; у пустого документа он становится корнем.
        A new element; in an empty document it becomes the root."""
        return wrap_node(LIB.beresta_node_append_element(self.handle, encode(name)), self.document)

    def append_text(self, text: str):
        return wrap_node(LIB.beresta_node_append_text(self.handle, encode(text)), self.document)

    def append_comment(self, text: str):
        return wrap_node(LIB.beresta_node_append_comment(self.handle, encode(text)), self.document)

    def append_cdata(self, text: str):
        return wrap_node(LIB.beresta_node_append_cdata(self.handle, encode(text)), self.document)

    def append_pi(self, target: str, data: Optional[str] = None):
        """Инструкция обработки <?target data?>. / A processing instruction <?target data?>."""
        pointer = LIB.beresta_node_append_pi(self.handle, encode(target), encode(data))
        return wrap_node(pointer, self.document)
