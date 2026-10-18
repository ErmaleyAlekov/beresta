"""Примесь: удаление, копирование и перенос поддерева. / Mixin: removing, copying and moving a subtree."""

from __future__ import annotations

from ._native import LIB, out_handle
from ._wrapping import transfer_wrappers, wrap_node
from .errors import check


class NodeStructureMixin:
    """Изменение структуры дерева. / Changing the tree structure."""

    def remove(self) -> None:
        """
        Удаление узла с потомками. После вызова этот объект и обёртки
        потомков недействительны — обращаться к ним нельзя.

        Removes the node with its descendants. After the call this object and the
        wrappers of its descendants are invalid — they must not be used.
        """
        check(LIB.beresta_node_remove(self.handle))

    def copy_to(self, parent):
        """Глубокая копия в конец детей parent (можно в другой документ).
        A deep copy appended to the children of parent (another document is allowed)."""
        result = out_handle()
        check(LIB.beresta_node_copy_to(self.handle, parent.handle, result))
        return wrap_node(result.value, parent.document)

    def move_to(self, parent) -> None:
        """
        Перенос (без копирования) в конец детей parent, в том числе в другой
        документ. Этот объект остаётся тем же узлом; после переноса его
        документ — parent.document, и закрытие прежнего документа узел не
        затрагивает. Обёртки потомков, полученные раньше, тоже переходят
        к новому документу. Корень документа и сам документ переносить
        нельзя (BerestaError HIERARCHY_VIOLATION / INVALID_ARGUMENT).

        Moves the node (without copying) to the end of the children of parent,
        including into another document. This object stays the same node; after the
        move its document is parent.document, and closing the former document does
        not affect the node. The wrappers of descendants obtained earlier move to the
        new document too. The document root and the document itself cannot be moved
        (BerestaError HIERARCHY_VIOLATION / INVALID_ARGUMENT).
        """
        source = self.document
        target = parent.document
        check(LIB.beresta_node_move_to(self.handle, parent.handle))
        if source is not target:
            transfer_wrappers(self, source, target)
