"""
Node — узел дерева документа.

Методы узла разнесены по примесям (mixin) по смыслу: содержимое,
навигация, дети, изменение дерева. Это принцип разделения интерфейсов
(ISP из SOLID): каждая примесь отвечает за одну обязанность и читается
отдельно, а класс Node лишь собирает их вместе.

Узел хранит ссылку на свой документ: пока жив хотя бы один узел, документ
не будет освобождён сборщиком мусора, а после document.close() любое
обращение к узлу даёт понятную ошибку вместо падения процесса.

Node — a node of the document tree.
The node methods are split into mixins by meaning: content,
navigation, children, tree changes. This is the interface segregation
principle (ISP from SOLID): each mixin has one responsibility and reads
separately, and the Node class only combines them.
A node keeps a reference to its document: while at least one node is alive,
the document is not freed by the garbage collector, and after document.close()
any access to the node gives a clear error instead of a process crash.
"""

from __future__ import annotations

from ._node_children import NodeChildrenMixin
from ._node_content import NodeContentMixin
from ._node_editing import NodeEditingMixin
from ._node_navigation import NodeNavigationMixin
from ._node_structure import NodeStructureMixin
from .languages import DOCUMENT_CLOSED, message_text


class Node(NodeContentMixin, NodeNavigationMixin, NodeChildrenMixin,
           NodeEditingMixin, NodeStructureMixin):
    """Узел документа: дескриптор Си-интерфейса и документ-владелец.
    A document node: the C interface handle and the owning document."""

    def __init__(self, pointer: int, document) -> None:
        self._pointer = pointer
        # Документ-владелец; move_to меняет его (см. _wrapping.transfer_wrappers)
        # The owner document; move_to changes it (see _wrapping.transfer_wrappers)
        self._document = document

    @property
    def handle(self) -> int:
        """Дескриптор для вызова Си-функций; проверяет, что документ жив.
        The handle for calling C functions; checks that the document is alive."""
        if self._document.closed:
            raise RuntimeError("Берёста / Beresta: " + message_text(DOCUMENT_CLOSED))
        return self._pointer

    @property
    def document(self):
        """Документ, которому принадлежит узел. / The document the node belongs to."""
        return self._document

    def __eq__(self, other: object) -> bool:
        """Два объекта-обёртки равны, если указывают на один узел.
        Two wrapper objects are equal if they point to the same node."""
        return isinstance(other, Node) and other._pointer == self._pointer

    def __hash__(self) -> int:
        return hash(self._pointer)
