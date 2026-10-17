"""
Node — узел дерева документа.

Методы узла разнесены по примесям (mixin) по смыслу: содержимое,
навигация, дети, изменение дерева. Это принцип разделения интерфейсов
(ISP из SOLID): каждая примесь отвечает за одну обязанность и читается
отдельно, а класс Node лишь собирает их вместе.

Узел хранит ссылку на свой документ: пока жив хотя бы один узел, документ
не будет освобождён сборщиком мусора, а после document.close() любое
обращение к узлу даёт понятную ошибку вместо падения процесса.
"""

from __future__ import annotations

from ._node_children import NodeChildrenMixin
from ._node_content import NodeContentMixin
from ._node_editing import NodeEditingMixin
from ._node_navigation import NodeNavigationMixin
from ._node_structure import NodeStructureMixin


class Node(NodeContentMixin, NodeNavigationMixin, NodeChildrenMixin,
           NodeEditingMixin, NodeStructureMixin):
    """Узел документа: дескриптор Си-интерфейса и документ-владелец."""

    def __init__(self, pointer: int, document) -> None:
        self._pointer = pointer
        self._document = document

    @property
    def handle(self) -> int:
        """Дескриптор для вызова Си-функций; проверяет, что документ жив."""
        if self._document.closed:
            raise RuntimeError("Берёста: документ уже закрыт, его узлы недействительны")
        return self._pointer

    @property
    def document(self):
        """Документ, которому принадлежит узел."""
        return self._document

    def __eq__(self, other: object) -> bool:
        """Два объекта-обёртки равны, если указывают на один узел."""
        return isinstance(other, Node) and other._pointer == self._pointer

    def __hash__(self) -> int:
        return hash(self._pointer)
