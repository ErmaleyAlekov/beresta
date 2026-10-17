"""Примесь: добавление новых узлов в конец детей."""

from __future__ import annotations

from typing import Optional

from ._native import LIB, encode
from ._wrapping import wrap_node


class NodeEditingMixin:
    """Каждый метод возвращает созданный узел — удобно строить дерево цепочкой."""

    def append_element(self, name: str):
        """Новый элемент; у пустого документа он становится корнем."""
        return wrap_node(LIB.beresta_node_append_element(self.handle, encode(name)), self.document)

    def append_text(self, text: str):
        return wrap_node(LIB.beresta_node_append_text(self.handle, encode(text)), self.document)

    def append_comment(self, text: str):
        return wrap_node(LIB.beresta_node_append_comment(self.handle, encode(text)), self.document)

    def append_cdata(self, text: str):
        return wrap_node(LIB.beresta_node_append_cdata(self.handle, encode(text)), self.document)

    def append_pi(self, target: str, data: Optional[str] = None):
        """Инструкция обработки <?target data?>."""
        pointer = LIB.beresta_node_append_pi(self.handle, encode(target), encode(data))
        return wrap_node(pointer, self.document)
