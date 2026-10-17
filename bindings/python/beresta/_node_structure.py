"""Примесь: удаление и копирование поддерева."""

from __future__ import annotations

from ._native import LIB, out_handle
from ._wrapping import wrap_node
from .errors import check


class NodeStructureMixin:
    """Изменение структуры дерева."""

    def remove(self) -> None:
        """
        Удаление узла с потомками. После вызова этот объект и обёртки
        потомков недействительны — обращаться к ним нельзя.
        """
        check(LIB.beresta_node_remove(self.handle))

    def copy_to(self, parent):
        """Глубокая копия в конец детей parent (можно в другой документ)."""
        result = out_handle()
        check(LIB.beresta_node_copy_to(self.handle, parent.handle, result))
        return wrap_node(result.value, parent.document)
